/**
 * @file microcontroller.h
 * @brief Microcontroller-specific functionality
 * 
 * Vendor-supplied driver for the microcontroller.
 * 
 * @author Microcontroller Engineering Team
 * @version 3.2.1
 * @date 2024-11-05
 * 
 * @copyright Copyright (c) 2024 Microcontroller Solutions Corp. All rights reserved.
 */

#ifndef MICROCONTROLLER_H
#define MICROCONTROLLER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup Microcontroller_Constants Microcontroller Constants
 * @{
 */

/** @} */

/** @defgroup Microcontroller Microcontroller Type Definitions
 * @{
 */

/** @} */

/** @defgroup Microcontroller_Functions Microcontroller API Functions
 * @{
 */

void delay_ms(uint16_t ms);
  
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* MICROCONTROLLER_H */
