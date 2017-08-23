.. image:: nectar.png
   :scale: 40%
   :align: right

CCS
===

As mentioned before, the backbone of the project is the 
EK-TM4C1294XL. To bring this device to life we will use
code composer studio. The latest version of CCS can be 
downloaded via the following link. Be sure to use one of 
the latest versions as pictured below.

----

.. image:: ccs.png

http://processors.wiki.ti.com/index.php/Download_CCS

-When selecting processor support, select the box starting with 
**TM4C12x**

.. image:: processor_support.png

----

CCS dependencies
----------------

For this project we will be using TI's real time operating
system, tivaware C series, and a couple other software packages. 

----

TI-RTOS
-------
	
- first open code composer studio(CCS)

- Select **View** -> **Resource Explorer**

.. image:: resource_exp.png

----

- In the right most panel, scroll down to TI-RTOS for Tiva C and click **Download and Install**

.. image:: tirtos.png

----


Tivaware
--------

- In the left window of the resource explorer, select **software** and click on the package starting with TM4C Arm Cortex

- In the right window, click **Download and Install**

.. image:: tivaware.png


- after the installation completes, restart CCS

----

Other packages
--------------

	- Once CCS has successfully restarted, the remaining packages to be installed should appear
	- Click **Install**

.. image:: packages.png

----

*Up Next:*

.. toctree::
   :maxdepth: 1

   InfluxDB
