//! @file
//! @author Marek Bel

#include "permanent_storage.h"
#include "mmctl.h"
#include "main.h"
#include <avr/eeprom.h>

#ifdef PSTOR_LOG
#define LOG(x) DBG_PRINT(x)
#else
#define LOG(x)
#endif

#ifdef PSTOR_TRACE
#define TRACE_LOG(x) DBG_PRINT(x)
#else
#define TRACE_LOG(x)
#endif

//! @brief EEPROM data layout
//!
//! Do not remove, reorder or change size of existing fields.
//! Otherwise values stored with previous version of firmware would be broken.
//! It is possible to add fields in the end of this struct, ensure that erased EEPROM is handled well.
typedef struct
{
  uint8_t eepromLengthCorrection; //!< legacy bowden length correction
  uint16_t eepromBowdenLen[5];    //!< Bowden length for each filament
}eeprom_t;

static eeprom_t * const eepromBase = reinterpret_cast<eeprom_t*>(0); //!< First EEPROM address
static const uint16_t eepromEmpty = 0xffff; //!< EEPROM content when erased
static const uint16_t eepromLengthCorrectionBase = 7900u; //!< legacy bowden length correction base
static const uint16_t eepromBowdenLenDefault = 8900u; //!< Default bowden length
static const uint16_t eepromBowdenLenMinimum = 6900u; //!< Minimum bowden length
static const uint16_t eepromBowdenLenMaximum = 10900u; //!< Maximum bowden length

//! @brief Is filament number valid?
//! @retval true valid
//! @retval false invalid
static bool validFilament(uint8_t filament)
{
  TRACE_LOG("Called, parameters:");
  TRACE_LOG(filament);
  TRACE_LOG("result:");

  if (filament < (sizeof(eeprom_t::eepromBowdenLen)/sizeof(eeprom_t::eepromBowdenLen[0])))
    {
      TRACE_LOG("true");
      return true;
    }
  else
    {
      TRACE_LOG("false");
      return false;
    }
}

//! @brief Is bowden length in valid range?
//! @param BowdenLength bowden length
//! @retval true valid
//! @retval false invalid
static bool validBowdenLen (const uint16_t BowdenLength)
{
  TRACE_LOG("Called, parameters:");
  TRACE_LOG(BowdenLength);
  TRACE_LOG("result:");
  if ((BowdenLength >= eepromBowdenLenMinimum) && BowdenLength <= eepromBowdenLenMaximum)
    {
      TRACE_LOG("true");
      return true;
    }
  else
    {
      TRACE_LOG("false");
      return false;
    }
}

//! @brief Get bowden length for active filament
//!
//! Returns stored value, doesn't return actual value when it is edited by increase() / decrease() unless it is stored.
//! @return stored bowden length
uint16_t BowdenLength::get()
{
  TRACE_LOG("Start");
  uint8_t filament = active_extruder;
  if (validFilament(filament))
    {
      uint16_t bowdenLength = eeprom_read_word(&(eepromBase->eepromBowdenLen[filament]));

      if (eepromEmpty == bowdenLength)
        {
          const uint8_t LengthCorrectionLegacy = eeprom_read_byte(&(eepromBase->eepromLengthCorrection));
          if (LengthCorrectionLegacy <= 200)
            {
              bowdenLength = eepromLengthCorrectionBase + LengthCorrectionLegacy * 10;
            }
        }
      if (validBowdenLen(bowdenLength))
        {
          TRACE_LOG("End, result:");
          TRACE_LOG(bowdenLength);
          return bowdenLength;
        }
    }

  TRACE_LOG("End, result:");
  TRACE_LOG(eepromBowdenLenDefault);
  return eepromBowdenLenDefault;
}


//! @brief Construct BowdenLength object which allows bowden length manipulation
//!
//! To be created on stack, new value is permanently stored when object goes out of scope.
//! Active filament and associated bowden length is stored in member variables.
BowdenLength::BowdenLength() : m_filament(active_extruder), m_length(BowdenLength::get())
{
  TRACE_LOG("Called");
}

//! @brief Increase bowden length
//!
//! New value is not stored immediately. See ~BowdenLength() for storing permanently.
//! @retval true passed
//! @retval false failed, it is not possible to increase, new bowden length would be out of range
bool BowdenLength::increase()
{
  TRACE_LOG("Called, result:");
  if ( validBowdenLen(m_length + stepSize))
    {
      m_length += stepSize;
      TRACE_LOG("true");
      return true;
    }
  TRACE_LOG("false");
  return false;
}

//! @brief Decrease bowden length
//!
//! New value is not stored immediately. See ~BowdenLength() for storing permanently.
//! @retval true passed
//! @retval false failed, it is not possible to decrease, new bowden length would be out of range
bool BowdenLength::decrease()
{
  TRACE_LOG("Called, result:");
  if ( validBowdenLen(m_length - stepSize))
    {
      m_length -= stepSize;
      TRACE_LOG("true");
      return true;
    }
  TRACE_LOG("false");
  return false;
}

//! @brief Store bowden length permanently.
BowdenLength::~BowdenLength()
{
  TRACE_LOG("Called");
  if (validFilament(m_filament))eeprom_update_word(&(eepromBase->eepromBowdenLen[m_filament]), m_length);
}

//! @brief Erase whole EEPROM
void BowdenLength::eraseAll()
{
  TRACE_LOG("Called");
  for (uint16_t i = 0; i < 1024; i++)
    {
      eeprom_update_byte((uint8_t*)i, static_cast<uint8_t>(eepromEmpty));
    }
}
