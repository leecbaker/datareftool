#pragma once

#include <boost/optional.hpp>

#include "allrefs.h"

extern boost::optional<RefRecords> refs;

void addUpdatedCommandThisFrame(RefRecord *);