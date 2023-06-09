/**@file
 * This file is part of the TASTE Linux Runtime.
 *
 * @copyright 2021 N7 Space Sp. z o.o.
 *
 * TASTE Linux Runtime was developed under a programme of,
 * and funded by, the European Space Agency (the "ESA").
 *
 * Licensed under the ESA Public License (ESA-PL) Permissive,
 * Version 2.3 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://essr.esa.int/license/list
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LINUX_SERIAL_CCSDS_H
#define LINUX_SERIAL_CCSDS_H

/**
 * @file     linux_serial_ccsds.h
 * @brief    Driver for TASTE with uses UART for communication
 */

#include <system_spec.h>
#include <stdint.h>
#include "dataview-uniq.h"

#ifdef __cplusplus
#include <cstddef>
#include <Thread.h>

extern "C"
{
#include <Broker.h>
#include <Escaper.h>
}

/**
 * @brief Structure for driver internal data.
 *
 * This structure is allocated by runtime and the pointer is passed to all driver functions.
 * The name of this structure shall match driver definition from ocarina_components.aadl
 * and has suffix '_private_data'.
 */
class linux_serial_ccsds_private_data final
{
  public:
    /**
     * @brief  Constructor.
     *
     * Construct empty object, which needs to be initialized using linux_serial_ccsds_private_data::init
     * before usage.
     */
    linux_serial_ccsds_private_data();

    /**
     * @brief  Destructor.
     *
     * Destruct created object
     */
    ~linux_serial_ccsds_private_data();

    /**
     * @brief Initialize driver.
     *
     * Driver needs to be initialized before start.
     *
     * @param bus_id         Identifier of the bus, which is used by driver
     * @param device_id      Identifier of the device
     * @param device_configuration Configuration of device
     * @param remote_device_configuration Configuration of remote device
     */
    void driver_init(const SystemBus bus_id,
                     const SystemDevice device_id,
                     const asn1SccRS485_LINUX_Conf_T* const device_configuration,
                     const asn1SccRS485_LINUX_Conf_T* const remote_device_configuration);
    /**
     * @brief Receive data from remote partitions.
     *
     * This function receives data from remote partition and sends it to the Broker.
     */
    void driver_poll();
    /**
     * @brief Send data to remote partition.
     *
     * @param data           The Buffer which data to send to connected remote partition
     * @param length         The size of the buffer
     */
    void driver_send(const uint8_t* data, const size_t length);
    /**
     * @brief Invoke mydriver broker receive packet function
     *
     * @param bus_id         Bus id of serial device
     * @param data           The Buffer which data to send to connected remote partition
     * @param data_size      The size of the buffer
     */
    static void driver_invoke_mydriver_broker_receive_packet
      (enum SystemBus bus_id, uint8_t* const data, const size_t data_size);

  private:
    static constexpr int DRIVER_THREAD_PRIORITY = 1;
    static constexpr int DRIVER_THREAD_STACK_SIZE = 65536;
    static constexpr size_t DRIVER_RECV_BUFFER_SIZE = 1 * 1024;
    static constexpr size_t ENCODED_PACKET_BUFFER_SIZE = 1 * 1024;
    static constexpr size_t DECODED_PACKET_BUFFER_SIZE = BROKER_BUFFER_SIZE;

    void driver_init_baudrate(const asn1SccRS485_LINUX_Conf_T* const device, int* cflags);
    void driver_init_character_size(const asn1SccRS485_LINUX_Conf_T* const device, int* cflags);
    void driver_init_parity(const asn1SccRS485_LINUX_Conf_T* const device, int* cflags);

    int m_serialFd;
    enum SystemBus m_serial_device_bus_id;
    enum SystemDevice m_serial_device_id;
    const asn1SccRS485_LINUX_Conf_T* m_serial_device_configuration{};
    const asn1SccRS485_LINUX_Conf_T* m_serial_remote_device_configuration{};
    taste::Thread m_thread;

    uint8_t m_recv_buffer[DRIVER_RECV_BUFFER_SIZE];
    uint8_t m_encoded_packet_buffer[ENCODED_PACKET_BUFFER_SIZE];
    uint8_t m_decoded_packet_buffer[DECODED_PACKET_BUFFER_SIZE];
    Escaper escaper{};
};

//namespace taste {
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize driver.
 *
 * Function is used by runtime to initialize the driver.
 *
 * @param private_data   Driver private data, allocated by runtime
 * @param bus_id         Identifier of the bus, which is used by driver
 * @param device_id      Identifier of the device
 * @param device_configuration Configuration of device
 * @param remote_device_configuration Configuration of remote device
 */
void LinuxSerialCcsdsInit(void* private_data,
                          const enum SystemBus bus_id,
                          const enum SystemDevice device_id,
                          const asn1SccRS485_LINUX_Conf_T* const device_configuration,
                          const asn1SccRS485_LINUX_Conf_T* const remote_device_configuration);

/**
 * @brief Function which implements receiving data from remote partition.
 *
 * Functions works in separate thread, which is initialized by LinuxSerialCcsdsSend
 *
 * @param private_data   Driver private data, allocated by runtime
 */
void LinuxSerialCcsdsPoll(void* private_data);

/**
 * @brief Send data to remote partition.
 *
 * Function is used by runtime.
 *
 * @param private_data   Driver private data, allocated by runtime
 * @param data           The Buffer which data to send to connected remote partition
 * @param length         The size of the buffer
 */
void LinuxSerialCcsdsSend(void* private_data, const uint8_t* const data, const size_t length);
//} // namespace taste

#ifdef __cplusplus
}
#endif

#endif
