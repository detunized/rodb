#ifndef database_h_included
#define database_h_included

#ifndef rodb_h_included
#error Please include "rodb.h", don't include Database.h directly
#endif

#include <fstream>
#include <vector>

namespace rodb
{

class Database
{
public:
	// Doesn't throw, returns NULL on error
	static Database *load(char const *filename)
	{
		try
		{
			return new Database(filename);
		}
		catch (std::bad_alloc const &)
		{
			return 0;
		}
		catch (std::runtime_error const &)
		{
			return 0;
		}
	}

	Database(char const *filename)
	{
		std::ifstream in(filename, std::ios::binary);
		if (in.fail())
			throw std::runtime_error("Cannot open input file");

		// Get file length
		in.seekg(0, std::ios::end);
		std::streampos size = in.tellg();
		in.seekg(0, std::ios::beg);

		// Read the entire file into memory
		data_.resize(size);
		in.read(&data_[0], size);
		
		check_integriry();
	}
	
	void dump(std::ostream &output_stream = std::cout) const
	{
		output_stream << *this;
	}
	
	Value root() const
	{
		return Value(header_ptr() + 1);
	}

private:
	struct Header
	{
		enum
		{
			SIGNATURE = 0x62646f72, // Should read 'rodb' when saved in little endian
			VERSION = 1,
		};
		
		uint32_t signature_;
		uint32_t version_;
	};
	
	Header const &header() const
	{
		return *header_ptr();
	}
	
	Header const *header_ptr() const
	{
		return reinterpret_cast<Header const *>(&data_[0]);
	}
	
	void check_integriry() const
	{
		if (header().signature_ != Header::SIGNATURE || header().version_ != Header::VERSION)
			throw std::runtime_error("Database integrity check failed");
	}
	
	std::vector<char> data_;

	// Beyond private ;)
private: 
	Database(Database const &)
	{
		throw std::logic_error("Cannot copy construct Database");
	}

	Database &operator =(Database const &)
	{
		throw std::logic_error("Cannot assign Database");
	}

	// BFF
	friend std::ostream &operator <<(std::ostream &stream, Database const&db);
};

inline std::ostream &operator <<(std::ostream &stream, Database const&db)
{
	return stream
		<< " Total size: " << db.data_.size() << "\n"
		<< "Header size: " << sizeof(db.header()) << "\n"
		<< "  Data size: " << db.data_.size() - sizeof(db.header()) << "\n"
		<< "  Signature: " << "0x" << std::hex << db.header().signature_ << std::dec << "\n"
		<< "    Version: " << db.header().version_ << "\n";
}

}

#endif
