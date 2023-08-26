#include "DisarrayPCH.hpp"

#include "core/filesystem/FileIO.hpp"

#include <filesystem>
#include <fstream>
#include <ostream>

#include "core/Formatters.hpp"
#include "core/Log.hpp"

namespace Disarray::FS {

template <> struct detail::GenericFileWriter<const void*>;
template <> struct detail::GenericFileWriter<const char*>;
template <> struct detail::GenericFileWriter<const unsigned*>;
template <> struct detail::GenericFileWriter<const void>;
template <> struct detail::GenericFileWriter<const char>;
template <> struct detail::GenericFileWriter<const unsigned>;

} // namespace Disarray::FS
