/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file YBRPARTIALToRGB.cpp
    \brief Implementation of the class YBRPARTIALToRGB.

*/

#include "exceptionImpl.h"
#include "YBRPARTIALToRGBImpl.h"
#include "dataHandlerImpl.h"
#include "dataSetImpl.h"
#include "imageImpl.h"


namespace imebra
{

namespace implementation
{

namespace transforms
{

namespace colorTransforms
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return the initial color space
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::string YBRPARTIALToRGB::getInitialColorSpace() const
{
    return "YBR_PARTIAL";
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return the final color space
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::string YBRPARTIALToRGB::getFinalColorSpace() const
{
    return "RGB";
}


} // namespace colorTransforms

} // namespace transforms

} // namespace implementation

} // namespace imebra

