#ifndef ABSL_STRINGS_INTERNAL_STR_FORMAT_FLOAT_CONVERSION_H_
#define ABSL_STRINGS_INTERNAL_STR_FORMAT_FLOAT_CONVERSION_H_

#include "Common/3rdparty/absl/strings/internal/str_format/extension.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace str_format_internal {

bool ConvertFloatImpl(float v, const FormatConversionSpecImpl &conv,
                      FormatSinkImpl *sink);

bool ConvertFloatImpl(double v, const FormatConversionSpecImpl &conv,
                      FormatSinkImpl *sink);

bool ConvertFloatImpl(long double v, const FormatConversionSpecImpl &conv,
                      FormatSinkImpl *sink);

}  // namespace str_format_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_STRINGS_INTERNAL_STR_FORMAT_FLOAT_CONVERSION_H_
