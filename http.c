#include "wqs.h"

#include <sys/socket.h>

#define HOSTNAME          /*"192.168.1.4:8086"*/ "192.168.1.237:8086"
#define REQUEST_URI       /*"/write?db=mydb"*/ "/write?db=db"
#define USER_AGENT        "Nectar WQN"
#define CONTENT_TYPE      "application/x-www-form-urlencode"
#define SENSOR            "WQN_input"
#define POD               "001"
#define HTTPTASKSTACKSIZE 4096

char buffer[1024];
char bufferlen[5];

HTTPCli_Struct cli;

/*
 *  ======== printError ========
 */
void printError(char *errString, int code)
{
    System_printf("Error! code = %d, desc = %s\n", code, errString);
    BIOS_exit(0);
}

void sendUpdate()
{
    bool moreFlag = false;
    char data[64];
    int ret;
    int len;

    //Call to get new values
    updateValues();

    System_printf("Sending a HTTP PUT request to '%s'\n", HOSTNAME);
    System_flush();

    sprintf(buffer,  "water_orp,pod=%s,sensor=%s value=%3.2f\nwater_ph,pod=%s,sensor=%s value=%3.2f\n"
                     "water_do,pod=%s,sensor=%s value=%3.2f\nwater_temp,pod=%s,sensor=%s value=%3.2f\n"
                     "water_elec,pod=%s,sensor=%s value=%3.2f",
                     POD, SENSOR, Orp, POD, SENSOR, Ph, POD, SENSOR, Do, POD, SENSOR, temp, POD, SENSOR, elec);
    sprintf(bufferlen, "%d", strlen(buffer));

    HTTPCli_Field fields[5] = {
        { HTTPStd_FIELD_NAME_HOST, HOSTNAME },
        { HTTPStd_FIELD_NAME_USER_AGENT, USER_AGENT },
        { HTTPStd_FIELD_NAME_CONTENT_TYPE, CONTENT_TYPE },
        { HTTPStd_FIELD_NAME_CONTENT_LENGTH, bufferlen},
        { NULL, NULL }
    };

    HTTPCli_setRequestFields(&cli, fields);

    ret = HTTPCli_sendRequest(&cli, HTTPStd_POST, REQUEST_URI, false);
    if (ret < 0) {
        printError("httpTask: send failed", ret);
    }

    ret = HTTPCli_sendRequestBody(&cli, buffer, strlen(buffer));
    if (ret < 0) {
        printError("httpTask: send body failed", ret);
    }

    ret = HTTPCli_getResponseStatus(&cli);
    if (ret != HTTPStd_OK && ret != HTTPStd_NO_CONTENT) {
        printError("httpTask: cannot get status", ret);
    }

    System_printf("HTTP Response Status Code: %d\n", ret);

    ret = HTTPCli_getResponseField(&cli, data, sizeof(data), &moreFlag);
    if (ret != HTTPCli_FIELD_ID_END) {
        printError("httpTask: response field processing failed", ret);
    }

    len = 0;
    do {
        ret = HTTPCli_readResponseBody(&cli, data, sizeof(data), &moreFlag);
        if (ret < 0) {
            printError("httpTask: response body processing failed", ret);
        }

        len += ret;
    } while (moreFlag);

    System_printf("Recieved %d bytes of payload\n", len);
    System_flush();

}

Void httpTask(UArg arg0, UArg arg1)
{
    int ret;

    struct sockaddr_in addr;

    System_printf("Attempting to perform address resolution for '%s'\n", HOSTNAME);
    System_flush();

    HTTPCli_construct(&cli);
    ret = HTTPCli_initSockAddr((struct sockaddr *)&addr, HOSTNAME, 0);
    if (ret < 0) {
        printError("httpTask: address resolution failed", ret);
        return;
    }
    System_printf("Attempting to connect to '%s'\n", HOSTNAME);
    System_flush();

    ret = -1;
    while (ret < 0)
    {
        ret = HTTPCli_connect(&cli, (struct sockaddr *)&addr, 0, NULL);
        if (ret < 0) {
            printError("httpTask: connect failed", ret);
        }
    }

    System_printf("Connected to '%s'\n", HOSTNAME);
    System_flush();

    //this will get modified later to handle drops and such
    while (1)
    {
        sendUpdate();
        Task_sleep(1000);
    }

    HTTPCli_disconnect(&cli);
    HTTPCli_destruct(&cli);
}

/*
 *  ======== netIPAddrHook ========
 *  This function is called when IP Addr is added/deleted
 */
void netIPAddrHook(unsigned int IPAddr, unsigned int IfIdx, unsigned int fAdd)
{
    static Task_Handle taskHandle;
    Task_Params taskParams;
    Error_Block eb;

    /* Create a HTTP task when the IP address is added */
    if (fAdd && !taskHandle) {
        Error_init(&eb);

        Task_Params_init(&taskParams);
        taskParams.stackSize = HTTPTASKSTACKSIZE;
        taskParams.priority = 1;
        taskHandle = Task_create((Task_FuncPtr)httpTask, &taskParams, &eb);
        if (taskHandle == NULL) {
            printError("netIPAddrHook: Failed to create HTTP Task\n", -1);
        }
    }
}
