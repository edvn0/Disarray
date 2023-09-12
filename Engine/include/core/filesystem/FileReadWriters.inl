namespace Detail {

template <class T> struct GenericFileWriter : FileWrite<T, GenericFileWriter<T>> {
	auto write_to_file_impl(std::string_view path_sv, std::size_t size, std::span<T> data) -> void
	{
		std::filesystem::path path { path_sv };
		std::ofstream stream { path };
		if (!stream) {
			return;
		}

		stream.write(Disarray::bit_cast<const char*>(data.data()), size);
	}
};

template <class T> struct GenericFileReader : FileRead<T, GenericFileReader<T>> {
	auto read_from_file_impl(std::string_view path_sv, std::vector<T>& out) -> bool
	{
		std::filesystem::path path { path_sv };
		std::ifstream stream { path, std::fstream::ate | std::fstream::in };
		if (!stream) {
			return false;
		}

		const auto size = stream.tellg();
		out.resize(size);

		stream.seekg(0);

		auto* cast = Disarray::bit_cast<char*>(out.data());
		stream.read(cast, size);

		return true;
	}

	auto read_from_file_impl(std::string_view path_sv, std::string& out) -> bool
	{
		std::filesystem::path path { path_sv };
		std::ifstream stream { path, std::fstream::ate | std::fstream::in };
		if (!stream) {
			return false;
		}

		const auto size = stream.tellg();
		out.resize(size);

		stream.seekg(0);
		stream.read(out.data(), size);

		return true;
	}
};

using CharFileRead = GenericFileReader<char>;
using UintFileRead = GenericFileReader<std::uint32_t>;
using UnsignedCharFileRead = GenericFileReader<unsigned char>;
using ConstCharFileRead = GenericFileReader<const char>;
using ConstUintFileRead = GenericFileReader<const std::uint32_t>;
using ConstUnsignedCharFileRead = GenericFileReader<const unsigned char>;

} // namespace Detail
