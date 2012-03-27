#include "blocxx/BLOCXX_config.h"
#include "blocxx/FileInformation.hpp"

/*
 * @author Kevin Harris
 */

namespace BLOCXX_NAMESPACE
{
	namespace FileSystem
	{
		FileInformation::FileInformation()
			: owner(UserId(-1))
			, group(gid_t(0))
			, size(0)
			, type(E_FILE_TYPE_UNKNOWN)
			, permissions(E_FILE_PERM_UNKNOWN)
		{
		}
	}

} // end namespace BLOCXX_NAMESPACE
