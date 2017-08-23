.. image:: nectar.png
   :scale: 40%
   :align: right

Chronograf
==========

Chronograf is the graphical interface that accesses data from
the InfluxDB database. Listed below is the link to the Chronograf
download for users on all platforms followed by a step-by-step 
installation for windows users with an already configured and tested
version of Chronograf.

----

**Chronograf link for all platforms:** https://portal.influxdata.com/downloads

----

**Below are the steps for setting up Chronograf for windows users:**

-Download InfluxDB dependencies :download:`here <../docs/chronograf-1.3.6.1.zip>`.

-unzip directory

-open chronograf.exe

-open your default web browser and enter: 'localhost:8888'

-click on "Configure Kapacitor"

-in the "Kapacitor URL" field enter: http://localhost:8086 to link the database
	
-in the "Name" field enter the name of your database

-connect and navigate to data explorer

.. image:: chronograf.png

----

*Up Next:*

.. toctree::
   :maxdepth: 1

   putty




	