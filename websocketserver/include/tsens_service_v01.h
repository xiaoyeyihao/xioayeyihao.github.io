/*
 *  Copyright (c) 2019 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef TSENS_QMI_SERVICE_SERVICE_01_H
#define TSENS_QMI_SERVICE_SERVICE_01_H
/**
  @file tsens_service_v01.h

  @brief This is the public header file which defines the tsens_qmi_service service Data structures.

  This header file defines the types and structures that were defined in
  tsens_qmi_service. It contains the constant values defined, enums, structures,
  messages, and service message IDs (in that order) Structures that were
  defined in the IDL as messages contain mandatory elements, optional
  elements, a combination of mandatory and optional elements (mandatory
  always come before optionals in the structure), or nothing (null message)

  An optional element in a message is preceded by a uint8_t value that must be
  set to true if the element is going to be included. When decoding a received
  message, the uint8_t values will be set to true or false by the decode
  routine, and should be checked before accessing the values that they
  correspond to.

  Variable sized arrays are defined as static sized arrays with an unsigned
  integer (32 bit) preceding it that must be set to the number of elements
  in the array that are valid. For Example:

  uint32_t test_opaque_len;
  uint8_t test_opaque[16];

  If only 4 elements are added to test_opaque[] then test_opaque_len must be
  set to 4 before sending the message.  When decoding, the _len value is set
  by the decode routine and should be checked so that the correct number of
  elements in the array will be accessed.

*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.14.7
   It was generated on: Fri Aug 23 2019 (Spin 0)
   From IDL File: tsens_service_v01.idl */

/** @defgroup tsens_qmi_service_qmi_consts Constant values defined in the IDL */
/** @defgroup tsens_qmi_service_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup tsens_qmi_service_qmi_enums Enumerated types used in QMI messages */
/** @defgroup tsens_qmi_service_qmi_messages Structures sent as QMI messages */
/** @defgroup tsens_qmi_service_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup tsens_qmi_service_qmi_accessor Accessor for QMI service object */
/** @defgroup tsens_qmi_service_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup tsens_qmi_service_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define TSENS_QMI_SERVICE_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define TSENS_QMI_SERVICE_V01_IDL_MINOR_VERS 0x01
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define TSENS_QMI_SERVICE_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define TSENS_QMI_SERVICE_V01_MAX_MESSAGE_ID 0x0004
/**
    @}
  */


/** @addtogroup tsens_qmi_service_qmi_consts
    @{
  */
/**
    @}
  */

/** @addtogroup tsens_qmi_service_qmi_enums
    @{
  */
typedef enum {
  TSENS_CMD_STATUS_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CMD_SUCCESS_V01 = 0,
  CMD_FAILURE_V01 = 1,
  TSENS_CMD_STATUS_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}tsens_cmd_status_v01;
/**
    @}
  */

/** @addtogroup tsens_qmi_service_qmi_enums
    @{
  */
typedef enum {
  TSENS_STATE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  ERROR_INTERNAL_V01 = -1,
  STATE_DISABLED_V01 = 0,
  STATE_ENABLED_V01 = 1,
  TSENS_STATE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}tsens_state_v01;
/**
    @}
  */

/** @addtogroup tsens_qmi_service_qmi_enums
    @{
  */
typedef enum {
  TSENS_IND_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  TSENS_DISABLED_V01 = 0,
  TSENS_ENABLED_V01 = 1,
  TIMER_EXPIRY_IMMINENT_V01 = 2,
  TSENS_IND_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}tsens_ind_v01;
/**
    @}
  */

/** @addtogroup tsens_qmi_service_qmi_messages
    @{
  */
/** Request Message; This command just registers the qmi client to server. */
typedef struct {

  /* Mandatory */
  uint8_t indication;
}tsens_service_register_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup tsens_qmi_service_qmi_messages
    @{
  */
/** Response Message; This command just registers the qmi client to server. */
typedef struct {

  /* Mandatory */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Mandatory */
  uint32_t duration;
}tsens_service_register_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup tsens_qmi_service_qmi_messages
    @{
  */
/** Request Message; This command is used by clients to know the current status of TSENS service */
typedef struct {

  /* Mandatory */
  uint8_t req;
}tsens_get_state_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup tsens_qmi_service_qmi_messages
    @{
  */
/** Response Message; This command is used by clients to know the current status of TSENS service */
typedef struct {

  /* Mandatory */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Mandatory */
  tsens_state_v01 current_state;
}tsens_get_state_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup tsens_qmi_service_qmi_messages
    @{
  */
/** Request Message; This is to mask/unmask TSENS to disable/enable auto shutdown. */
typedef struct {

  /* Mandatory */
  tsens_state_v01 state;

  /* Optional */
  uint8_t timeout_valid;  /**< Must be set to true if timeout is being passed */
  uint32_t timeout;
}tsens_set_state_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup tsens_qmi_service_qmi_messages
    @{
  */
/** Response Message; This is to mask/unmask TSENS to disable/enable auto shutdown. */
typedef struct {

  /* Mandatory */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Mandatory */
  tsens_cmd_status_v01 status;
}tsens_set_state_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup tsens_qmi_service_qmi_messages
    @{
  */
/** Indication Message; This is the notification sent by QMI service to clients regarding imminent auto-enable or state change events */
typedef struct {

  /* Mandatory */
  tsens_ind_v01 notify_state;
}tsens_broadcast_indication_msg_v01;  /* Message */
/**
    @}
  */

/* Conditional compilation tags for message removal */
//#define REMOVE_TSENS_GET_STATE_V01
//#define REMOVE_TSENS_SERVICE_BROADCAST_IND_V01
//#define REMOVE_TSENS_SERVICE_REGISTRATION_V01
//#define REMOVE_TSENS_SET_STATE_V01

/*Service Message Definition*/
/** @addtogroup tsens_qmi_service_qmi_msg_ids
    @{
  */
#define TSENS_SERVICE_REGISTER_REQ_V01 0x0001
#define TSENS_SERVICE_REGISTER_RESP_V01 0x0001
#define TSENS_SET_STATE_REQ_V01 0x0002
#define TSENS_SET_STATE_RESP_V01 0x0002
#define TSENS_GET_STATE_REQ_V01 0x0003
#define TSENS_GET_STATE_RESP_V01 0x0003
#define TSENS_SERVICE_BROADCAST_IND_V01 0x0004
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro tsens_qmi_service_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type tsens_qmi_service_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );

/** This macro should be used to get the service object */
#define tsens_qmi_service_get_service_object_v01( ) \
          tsens_qmi_service_get_service_object_internal_v01( \
            TSENS_QMI_SERVICE_V01_IDL_MAJOR_VERS, TSENS_QMI_SERVICE_V01_IDL_MINOR_VERS, \
            TSENS_QMI_SERVICE_V01_IDL_TOOL_VERS )
/**
    @}
  */


#ifdef __cplusplus
}
#endif
#endif

