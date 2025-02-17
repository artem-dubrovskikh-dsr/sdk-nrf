.. _asset_tracker_v2_sensor_module:

Sensor module
#############

.. contents::
   :local:
   :depth: 2

The sensor module interacts with external sensors present on the `Thingy:91 <Thingy:91 product page>`_.
It collects environmental data and detects motion over a set threshold value.

Features
********

This section documents the various features implemented by the module.

Sensor types
============

The following table lists the sensors and sensor data types supported by the module:

+-------------------------+-----------------+
| Sensor data type        | External device |
+=========================+=================+
| Humidity                | `BME680`_       |
+-------------------------+-----------------+
| Temperature             | `BME680`_       |
+-------------------------+-----------------+
| Atmospheric Pressure    | `BME680`_       |
+-------------------------+-----------------+
| Air Quality (BSEC only) | `BME680`_       |
+-------------------------+-----------------+
| Acceleration            | `ADXL362`_      |
+-------------------------+-----------------+

The module controls and collects data from the sensors by interacting with their :ref:`device drivers <device_model_api>` using :ref:`Zephyr's generic sensor API <sensor_api>`.

Data sampling
=============

When the module receives an :c:enum:`APP_EVT_DATA_GET` event and the :c:enum:`APP_DATA_ENVIRONMENTAL` type is present in the ``app_data`` list carried in the event, it will sample data.
When data sampling has been carried out, the :c:enum:`SENSOR_EVT_ENVIRONMENTAL_DATA_READY` event is sent from the module with the sampled environmental sensor values.

.. note::
   The nRF9160 DK does not have any external sensors.
   If the sensor module is queried for sensor data when building for the DK, the event :c:enum:`SENSOR_EVT_ENVIRONMENTAL_NOT_SUPPORTED` is sent out by the module
   upon data sampling.

Motion detection
================

Motion is detected when acceleration in either X, Y or Z plane exceeds the configured threshold value.
The threshold is set in one of the following two ways:

* When receiving the :c:enum:`DATA_EVT_CONFIG_INIT` event after boot.
  This event contains the default threshold value set by the :ref:`CONFIG_DATA_ACCELEROMETER_THRESHOLD <CONFIG_DATA_ACCELEROMETER_THRESHOLD>` option or retrieved from flash.
* When receiving the :c:enum:`DATA_EVT_CONFIG_READY` event.
  This occurs when a new threshold value has been updated from cloud.

Both events contain an accelerometer threshold value ``accelerometer_threshold`` in m/s2, present in the event structure.

Motion detection in the module is enabled and disabled in turn to control the number of :c:enum:`SENSOR_EVT_MOVEMENT_DATA_READY` events that are sent out by the sensor module.
This functionality acts as flow control.
It prevents the sensor module from sending events to the rest of the application if the device is accelerating over the threshold for extended periods of time.

The applicaton module controls this behavior through the :c:enum:`APP_EVT_ACTIVITY_DETECTION_ENABLE` and :c:enum:`APP_EVT_ACTIVITY_DETECTION_DISABLE` events.
The sensor module will only send out a :c:enum:`SENSOR_EVT_MOVEMENT_DATA_READY` event if it detects movement and activity detection is enabled.

.. note::
   The DK does not have an external accelerometer.
   However, you can use **Button 2** on the DK to trigger movement for testing purposes.

.. note::
   The accelerometer available on the Thingy:91 needs detailed tuning for each use case to determine reliably which readings are considered as motion.
   This is beyond the scope of the general asset tracker framework this application provides.
   Therefore, the readings are not transmitted to the cloud and are only used to detect a binary active and inactive state.

.. _bosch_software_environmental_cluster_library:

Bosch Software Environmental Cluster (BSEC) library
===================================================

The sensor module supports integration with the BSEC signal processing library using the external sensors, internal convenience API.
If enabled, the BSEC library is used instead of the BME680 Zephyr driver to provide sensor readings from the BME680 for temperature, humidity, and atmospheric pressure.
In addition, the BSEC driver provides an additional sensor reading, indoor air quality (IAQ), which is a metric given in between 0-500 range, that estimates the air quality of the environment.

As the BSEC library requires a separate license, it is not a default part of |NCS|, but can be downloaded externally and imported into the |NCS| source tree.

Perform the following steps to enable BSEC:

1. Download the BSEC library, using the `Bosch BSEC`_ link.
#. Extract and store the folder containing the library contents in the path specified by :ref:`CONFIG_EXTERNAL_SENSORS_BME680_BSEC_PATH <CONFIG_EXTERNAL_SENSORS_BME680_BSEC_PATH>` option or update the path configuration to reference the library location.
#. Disable the Zephyr BME680 driver by setting :kconfig:option:`CONFIG_BME680` to false.
#. Enable the external sensors API BSEC integration layer by enabling :ref:`CONFIG_EXTERNAL_SENSORS_BME680_BSEC <CONFIG_EXTERNAL_SENSORS_BME680_BSEC>` option.

Air quality readings are provided with the :c:enumerator:`SENSOR_EVT_ENVIRONMENTAL_DATA_READY` event.

To check and configure the BSEC configuration options, see :ref:`external_sensor_API_BSEC_configurations` section.

Configuration options
*********************

.. _CONFIG_SENSOR_THREAD_STACK_SIZE:

CONFIG_SENSOR_THREAD_STACK_SIZE - Sensor module thread stack size
   This option configures the sensor module's internal thread stack size.

.. _CONFIG_DATA_ACCELEROMETER_THRESHOLD:

CONFIG_DATA_ACCELEROMETER_THRESHOLD
   This configuration sets the accelerometer threshold value.

.. _external_sensor_API_BSEC_configurations:

External sensors API BSEC configurations
========================================

.. _CONFIG_EXTERNAL_SENSORS_BME680_BSEC:

CONFIG_EXTERNAL_SENSORS_BME680_BSEC
   This option configures the Bosch BSEC library for the BME680.

.. _CONFIG_EXTERNAL_SENSORS_BME680_BSEC_PATH:

CONFIG_EXTERNAL_SENSORS_BME680_BSEC_PATH
   This option sets the path for the Bosch BSEC library folder.

.. _CONFIG_EXTERNAL_SENSORS_BSEC_SAMPLE_MODE_ULTRA_LOW_POWER:

CONFIG_EXTERNAL_SENSORS_BSEC_SAMPLE_MODE_ULTRA_LOW_POWER
   This option configures the BSEC ultra Low Power Mode. In this mode, the BME680 is sampled every 300 seconds.

.. _CONFIG_EXTERNAL_SENSORS_BSEC_SAMPLE_MODE_LOW_POWER:

CONFIG_EXTERNAL_SENSORS_BSEC_SAMPLE_MODE_LOW_POWER
   This option configures BSEC Low Power Mode. In this mode, the BME680 is sampled every 3 seconds.

.. _CONFIG_EXTERNAL_SENSORS_BSEC_SAMPLE_MODE_CONTINUOUS:

CONFIG_EXTERNAL_SENSORS_BSEC_SAMPLE_MODE_CONTINUOUS
  This option configures BSEC continuous Mode. In this mode, the BME680 is sampled every second.

.. _CONFIG_EXTERNAL_SENSORS_BSEC_TEMPERATURE_OFFSET:

CONFIG_EXTERNAL_SENSORS_BSEC_TEMPERATURE_OFFSET
   This option configures BSEC temperature offset in degree celsius multiplied by 100.

Module states
*************

The sensor module has an internal state machine with the following states:

* ``STATE_INIT`` - The initial state of the module in which it awaits its initial configuation from the data module.
* ``STATE_RUNNING`` - The module is initialized and can be queried for sensor data. It will also send :c:enum:`SENSOR_EVT_MOVEMENT_DATA_READY` on movement.
* ``STATE_SHUTDOWN`` - The module has been shut down after receiving a request from the utility module.

State transitions take place based on events from other modules, such as the app module, data module, and utility module.

Module events
*************

The :file:`asset_tracker_v2/src/events/sensor_module_event.h` header file contains a list of various events sent by the module.

Dependencies
************

This module uses the following |NCS| libraries and drivers:

* :ref:`Generic sensor API <sensor_api>`
* :ref:`adxl362`
* :ref:`bme680`

API documentation
*****************

| Header file: :file:`asset_tracker_v2/src/events/sensor_module_event.h`
| Source files: :file:`asset_tracker_v2/src/events/sensor_module_event.c`
                :file:`asset_tracker_v2/src/modules/sensor_module.c`

.. doxygengroup:: sensor_module_event
   :project: nrf
   :members:
