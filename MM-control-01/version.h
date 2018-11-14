//! @file

#ifndef VERSION_H_
#define VERSION_H_

#define FW_VERSION 102 //!< example: 103 means version 1.0.3
#define FW_BUILDNR 171 //!< number of commits in 'master'
#define FW_HASH "df02735ce9ca7b7b27b3e465bc80446c000ac42a"
//! @macro FW_LOCAL_CHANGES
//! @val 0 no changes in tracked local files
//! @val 1 some local git tracked files has been changed
#include "dirty.h"

#endif //VERSION_H_