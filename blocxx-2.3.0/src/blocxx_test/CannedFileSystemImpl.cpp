#include "blocxx/BLOCXX_config.h"
#include "blocxx_test/CannedFileSystemImpl.hpp"
#include "blocxx_test/TextUtils.hpp"
#include "blocxx_test/LogUtils.hpp"
#include "blocxx/Logger.hpp"
#include "blocxx/Format.hpp"

using namespace blocxx;

namespace BLOCXX_NAMESPACE
{
	namespace Test
	{
		namespace CannedFSImpl
		{
			namespace // anonymous
			{
				const GlobalString COMPONENT_NAME = BLOCXX_LAZY_GLOBAL_INIT("blocxx.test.CannedFileSystem");

				// Something to prevent infinite recursion when reading from symlinks...
				const size_t MAX_SYMLINK_DEPTH = 32;
				const size_t MAX_PATH_DEPTH = 32;
			} // end anonymous namespace

			namespace // anonymous
			{
				class FSTreeEntry;
				typedef Reference<FSTreeEntry> FSTreeEntryRef;

				// This struct is a filesystem tree entry.  It represents a file or
				// directory.  The way to distinguish between a file and a directory
				// is that a file will have a non-null file entry.  This seems
				// hackish, but is the easiset wey I could come up with to write a
				// very simple directory tree structure.
				class FSTreeEntry
				{
				public:
					FSTreeEntry(const String& name) : m_fullpath(name)
					{
					}
					virtual ~FSTreeEntry() { }

					// If this is a directory entry, there will be some number of
					// children (may be 0, but the current exposed interface has no way
					// to create an empty directory).
					// This map maps file/directory names (relative to this directory)
					// to more tree entries.
					typedef SortedVectorMap<String, FSTreeEntryRef> MapType;

					virtual bool isDirectory() const { return false; }
					virtual bool isLink() const { return false; }
					virtual bool haveChildren() const { return false; }
					virtual MapType& getChildren() { BLOCXX_THROW( blocxx::FileSystemException, Format("File \"%1\" has no children", m_fullpath).c_str() ); }

					String getFullPath() { return m_fullpath; }
					virtual FileInformation getFileInfo() const = 0;

					virtual FSEntryRef getFileEntry() const { return FSEntryRef(); }

				private:
					// The name (full path) of the file entry.
					String m_fullpath;
				};

				class FSTreeFileEntry : public FSTreeEntry
				{
				public:
					FSTreeFileEntry(const String& name,
						const FSEntryRef& entry = FSEntryRef())
						: FSTreeEntry(name)
						, m_file(entry)
					{
					}
					virtual ~FSTreeFileEntry() { }

					FSEntryRef getFileEntry() const { return m_file; }

					FileInformation getFileInfo() const { return m_file->getFileInfo(); }
					bool isLink() const { return m_file->getFileInfo().type == FileInformation::E_FILE_SYMLINK; }
				private:
					FSEntryRef m_file;
				};

				class FSTreeDirEntry : public FSTreeEntry
				{
				public:
					FSTreeDirEntry(const String& name) : FSTreeEntry(name) { }
					virtual ~FSTreeDirEntry() { }

					bool isDirectory() const { return true; }
					bool haveChildren() const { return !m_children.empty(); }
					MapType& getChildren() { return m_children; }

					FileInformation getFileInfo() const
					{
						FileInformation info;
						info.type = FileInformation::E_FILE_DIRECTORY;
						info.owner = 0;
						return info;
					}

				private:
					MapType m_children;
				};
			}

			// This class is not exposed.  It is just an implementation of the
			// FileSystemMockObject.  Functionality will be added as needed.
			class CannedFileSystemObject : public FileSystemMockObject
			{
			public:

				enum EParentDirectoryCreationFlag { E_DO_NOT_CREATE_PARENT, E_CREATE_PARENT_DIRECTORY };

				CannedFileSystemObject(const String& name)
					: m_root( new FSTreeDirEntry("/") )
					, m_name(name)
				{
				}
				virtual ~CannedFileSystemObject() { }

				FSTreeEntryRef fullyResolvePath(FSTreeEntryRef base, const StringArray& relativeComponents
					, EParentDirectoryCreationFlag createFlag = E_DO_NOT_CREATE_PARENT
					, size_t recurseDepth = 0, size_t linkDepth = 0);
				FSTreeEntryRef fullyResolvePath(const String& path
					, EParentDirectoryCreationFlag createFlag = E_DO_NOT_CREATE_PARENT
					, size_t recurseDepth = 0, size_t linkDepth = 0);

				// Find or create (if enabled) a directory.  If the directory is
				// not found and creation is disabled, then NULL will be returned.
				FSTreeEntryRef findDirectory(const String& path
					, EParentDirectoryCreationFlag createFlag = E_DO_NOT_CREATE_PARENT);

				// Add a file to the mock filesystem.  This will create any needed
				// parent directories.
				void addFile(const FSEntryRef& file);

				// Get an entry from the directory tree.  If no matching path
				// found, this will return a null entry.
				FSTreeEntryRef getTreeEntry(const String& path);
				FSEntryRef getFileRef(const String& path);

				// Add a child to the directory given by path.  Returns the child
				// it was passed, or FSTreeEntryRef() if the base path could not be
				// located.
				FSTreeEntryRef addChild(const String& path, FSTreeEntryRef child);

				// Add a child to the entry given by parent.  The name of the child
				// will be basename(child->getFullPath()).  Returns the child it
				// was passed or FSTreeEntryRef() if the parent was NULL.
				FSTreeEntryRef addChild(FSTreeEntryRef parent, FSTreeEntryRef child);

				// Functions inherited from FileSystemMockObject...
				virtual String dirname(const String& path);
				virtual String basename(const String& path);
				FileSystem::FileInformation getFileInformation(const String& filename);
				virtual bool canRead(const String& path);
				virtual bool canWrite(const String& path);
				virtual bool isDirectory(const String& path);
				virtual bool isLink(const String& path);
				virtual bool exists(const String& path);
				virtual bool removeFile(const String& path);
				virtual String getFileContents(const String& filename);
				virtual StringArray getFileLines(const String& filename);
				virtual bool getDirectoryContents(const String& path, StringArray& dirEntries);
				virtual String readSymbolicLink(const String& path);

			private:
				FSTreeEntryRef m_root;

				const String m_name;

				String doGetFileContents(const String& filename, size_t depth);
			};

#define LOG_DEBUG3(X) STANDARD_LOG_DEBUG3( "fullyResolvePath(" + m_name + "): ", (X) )

			FSTreeEntryRef CannedFileSystemObject::fullyResolvePath(FSTreeEntryRef base
				, const StringArray& relativeComponents, EParentDirectoryCreationFlag createFlag
				, size_t pathDepth, size_t linkDepth)
			{
				Logger logger(COMPONENT_NAME);
				if( !base )
				{
					LOG_DEBUG3( Format("Bad base directory given.  Relative Components=\"%1\""
							, TextUtils::untokenize(relativeComponents, "\", \"") ));
					return FSTreeEntryRef();
				}

				LOG_DEBUG3(Format("Resolving from start point \"%1\"", base->getFullPath()));
				if( base->isLink() )
				{
					if( linkDepth >= MAX_SYMLINK_DEPTH )
					{
						LOG_DEBUG3(Format("Maximum symlink depth exceeded for \"%1\".", base->getFullPath()));
						return FSTreeEntryRef();
					}

					// Resolve the link.
					LOG_DEBUG3(Format("Resolving symlink \"%1\"", base->getFullPath()));
					FSTreeEntryRef fileEntry = fullyResolvePath(base->getFileEntry()->contents()
						, createFlag, pathDepth, linkDepth + 1);

					LOG_DEBUG3(Format("Link \"%1\" resolved as \"%2\"", base->getFullPath()
							, fileEntry?fileEntry->getFullPath():String()));
					// Resolve the file to which the link points
					return fullyResolvePath(fileEntry, relativeComponents, createFlag, pathDepth, linkDepth);
				}

				// With no more directory compontents to locate, the current entry
				// is acceptable (directory or file).  It can't be a link because
				// the above tests would have handled that.
				if ( relativeComponents.empty() )
				{
					return base;
				}

				// If there are more relative components, the path is not yet
				// complete and the current entry must be a directory.
				if( !base->isDirectory() )
				{
					LOG_DEBUG3( Format("Non-directory found where directory is required: \"%1\""
							, base->getFullPath()) );
					return FSTreeEntryRef();
				}

				LOG_DEBUG3(Format("Base point is a directory, will check: \"%1\"", base->getFullPath()));

				String childName = *relativeComponents.begin();

				if( pathDepth >= MAX_PATH_DEPTH )
				{
					LOG_DEBUG3(Format("Maximum depth exceeded for base directory \"%1\" with child \"%2\"."
							, base->getFullPath(), childName));
					return FSTreeEntryRef();
				}

				String fullChildPath = TextUtils::removeRedundantSeparators(base->getFullPath() + "/" + childName);
				StringArray remainingComponents(relativeComponents.begin() + 1, relativeComponents.end());


				if( childName == "." || childName == "/" )
				{
					LOG_DEBUG3(Format("Recursing with existing directory (%1) for entry \"%2\""
							, base->getFullPath(), childName));

					return fullyResolvePath(base, remainingComponents
						, createFlag, pathDepth + 1, linkDepth);
				}

				FSTreeEntry::MapType& children = base->getChildren();
				FSTreeEntry::MapType::const_iterator fileEntry = children.find(childName);
				if( fileEntry != children.end() )
				{
					LOG_DEBUG3( Format("Found child entry for \"%1\", recursing.", fullChildPath) );

					return fullyResolvePath(fileEntry->second, remainingComponents
						, createFlag, pathDepth + 1, linkDepth);
				}
				else if( createFlag == E_CREATE_PARENT_DIRECTORY )
				{
					LOG_DEBUG3( Format("Creating directory entry for \"%1\"", fullChildPath) );
					FSTreeEntryRef nextEntry = addChild(base, FSTreeEntryRef(new FSTreeDirEntry(fullChildPath)));

					LOG_DEBUG3( Format("Created child entry for \"%1\", recursing.", fullChildPath) );

					return fullyResolvePath(nextEntry, remainingComponents
						, createFlag, pathDepth + 1, linkDepth);
				}
				else
				{
					LOG_DEBUG3(Format("Base directory \"%1\" has no child \"%2\" and creation is disallowed."
							, base->getFullPath(), childName));
					return FSTreeEntryRef();
				}
			}

			FSTreeEntryRef CannedFileSystemObject::fullyResolvePath(const String& path, EParentDirectoryCreationFlag createFlag, size_t pathDepth, size_t linkDepth)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("Resolving path \"%1\"", path) );

				FSTreeEntryRef retval = fullyResolvePath(m_root, path.tokenize("/")
					, createFlag, pathDepth, linkDepth);

				if( retval )
				{
					LOG_DEBUG3( Format("Path \"%1\" fully resolved as \"%2\"", path, retval->getFullPath()) );
				}
				else
				{
					LOG_DEBUG3( Format("Path \"%1\" failed to resolve.", path) );
				}
				return retval;
			}
#undef LOG_DEBUG3

#define LOG_DEBUG3(X) STANDARD_LOG_DEBUG3( "findDirectory(" + m_name + "): ", (X) )
			FSTreeEntryRef CannedFileSystemObject::findDirectory(const String& path
				, EParentDirectoryCreationFlag create)
			{
				Logger logger(COMPONENT_NAME);

				LOG_DEBUG3( Format("Searching for directory named \"%1\" (create=%2)", path, create) );

				FSTreeEntryRef entry = fullyResolvePath(path, create);

				if( entry )
				{
					if( !entry->isDirectory() )
					{
						LOG_DEBUG3( Format("Oops.  Found a non-directory (\"%1\") while searching for directory \"%2\" ..."
								, entry->getFullPath(), path) );
						return FSTreeEntryRef();
					}
					else
					{
						LOG_DEBUG3( Format("We now have an entry for the desired path (\"%1\")", path) );
					}
				}
				else
				{
					LOG_DEBUG3( Format("Found no directory entry for \"%1\"", path) );
				}

				return entry;
			}

#undef LOG_DEBUG3


#define LOG_DEBUG3(X) STANDARD_LOG_DEBUG3( "getTreeEntry(" + m_name + "): ", (X) )
			FSTreeEntryRef CannedFileSystemObject::getTreeEntry(const String& path)
			{
				Logger logger(COMPONENT_NAME);

				LOG_DEBUG3(Format("getting a tree entry for \"%1\"", path));

				// This must be done in two steps, since the directory must be
				// fully resolved, but we want to retain any symlink properties of
				// the file, should it be a symlink.
				String directory = dirname(path);
				String filename = basename(path);
				FSTreeEntryRef basedir = findDirectory(directory);

				if( basedir )
				{
					if( filename == "." || filename == "/" )
					{
						LOG_DEBUG3(Format("Using existing directory (%1) for entry \"%2\"", directory, filename));
						return basedir;
					}

					const FSTreeEntry::MapType& children = basedir->getChildren();
					FSTreeEntry::MapType::const_iterator fileEntry = children.find(filename);

					if( fileEntry != children.end() )
					{
						LOG_DEBUG3(Format("Found child entry \"%1\" in base directory \"%2\"", filename, basedir->getFullPath()));
						return fileEntry->second;
					}
					else
					{
						LOG_DEBUG3(Format("Directory \"%1\" has no child \"%2\"", basedir->getFullPath(), filename));
					}
				}
				else
				{
					LOG_DEBUG3(Format("Found no directory \"%1\"", directory));
				}
				return FSTreeEntryRef();
			}

#undef LOG_DEBUG3

#define LOG_DEBUG3(X) STANDARD_LOG_DEBUG3( "getFileRef(" + m_name + "): ", (X) )

			FSEntryRef CannedFileSystemObject::getFileRef(const String& path)
			{
				FSTreeEntryRef entry = fullyResolvePath(path);

				if( entry )
				{
					return entry->getFileEntry();
				}

				return FSEntryRef();
			}

#undef LOG_DEBUG3

#define LOG_DEBUG3(X) STANDARD_LOG_DEBUG3( "addChild(" + m_name + "): ", (X) )
			FSTreeEntryRef CannedFileSystemObject::addChild(const String& path, FSTreeEntryRef child)
			{
				return addChild(findDirectory(path), child);
			}

			FSTreeEntryRef CannedFileSystemObject::addChild(FSTreeEntryRef parent, FSTreeEntryRef child)
			{
				Logger logger(COMPONENT_NAME);

				String filename = basename(child->getFullPath());

				if( !parent )
				{
					LOG_DEBUG3( Format("Cannot insert child into non-existent directory: \"%1\""
							, child->getFullPath()) );
					return FSTreeEntryRef();
				}

				// Add it to the containing directory.
				LOG_DEBUG3( Format("Inserting file \"%1\" in directory \"%2\""
						, filename, parent->getFullPath()) );
				parent->getChildren()[filename] = child;

				return child;
			}
#undef LOG_DEBUG3

#define LOG_DEBUG3(X) STANDARD_LOG_DEBUG3( "addFile(" + m_name + "): ", (X) )
			void CannedFileSystemObject::addFile(const FSEntryRef& file)
			{
				Logger logger(COMPONENT_NAME);

				String filename = file->path();
				LOG_DEBUG3( Format("Handing requested add for file \"%1\"", filename) );

				String directoryPortion = dirname(filename);

				if (file->getFileType() == FileInformation::E_FILE_DIRECTORY)
				{
					LOG_DEBUG3("File was a directory.  Not adding ref but creating new directory.");
					findDirectory(filename, E_CREATE_PARENT_DIRECTORY);
					return;
				}

				addChild(findDirectory(directoryPortion, E_CREATE_PARENT_DIRECTORY)
					, FSTreeEntryRef(new FSTreeFileEntry(filename, file)));
			}
#undef LOG_DEBUG3

#define LOG_DEBUG3(X) STANDARD_LOG_DEBUG3( "MockFileOperations(" + m_name + "): ", (X) )
			// Implementation stolen from FileSystem::dirname()
			String CannedFileSystemObject::dirname(const String& filename)
			{
				// skip over trailing slashes
				if (filename.length() == 0)
				{
					return ".";
				}
				size_t lastSlash = filename.length() - 1;

				while (lastSlash > 0
					&& filename[lastSlash] == BLOCXX_FILENAME_SEPARATOR_C)
				{
					--lastSlash;
				}

				lastSlash = filename.lastIndexOf(BLOCXX_FILENAME_SEPARATOR_C, lastSlash);

				if (lastSlash == String::npos)
				{
					return ".";
				}

				while (lastSlash > 0 && filename[lastSlash - 1] == BLOCXX_FILENAME_SEPARATOR_C)
				{
					--lastSlash;
				}

				if (lastSlash == 0)
				{
					return BLOCXX_FILENAME_SEPARATOR;
				}

				return filename.substring(0, lastSlash);
			}

			String CannedFileSystemObject::basename(const String& filename)
			{
				if (filename.length() == 0)
				{
					return filename;
				}
				size_t end = filename.length() - 1;

				while (end > 0
					&& filename[end] == BLOCXX_FILENAME_SEPARATOR_C)
				{
					--end;
				}

				if (end == 0 && filename[0] == BLOCXX_FILENAME_SEPARATOR_C)
				{
					return BLOCXX_FILENAME_SEPARATOR;
				}

				if (end == filename.length() - 1)
				{
					end = String::npos;
				}
				size_t beg = filename.lastIndexOf(BLOCXX_FILENAME_SEPARATOR_C, end);

				if (beg == String::npos)
				{
					beg = 0;
				}
				else
				{
					++beg;
				}
				size_t len = end == String::npos ? end : ++end - beg;
				return filename.substring(beg, len);
			}

			FileSystem::FileInformation CannedFileSystemObject::getFileInformation(const String& filename)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("getFileInformation() called for \"%1\"", filename) );

				FileSystem::FileInformation info;

				FSTreeEntryRef entry = getTreeEntry(filename);
				if ( entry )
				{
					LOG_DEBUG3( Format("getFileInformation() found file entry for \"%1\".  Obtaining information", filename) );
					info = entry->getFileInfo();
				}
				else
				{
					LOG_DEBUG3( Format("getFileInformation() found no entry for \"%1\"", filename) );
				}
				LOG_DEBUG3("getFileInformation() done");
				return info;
			}

			bool CannedFileSystemObject::isDirectory(const String& path)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("isDirectory() called for \"%1\"", path) );

				FSTreeEntryRef dirEntry = findDirectory(path);

				if ( dirEntry && dirEntry->isDirectory() )
				{
					LOG_DEBUG3( Format("isDirectory() returning true for \"%1\"", path) );
					return true;
				}
				LOG_DEBUG3( Format("isDirectory() returning false for \"%1\"", path) );
				return false;
			}

			bool CannedFileSystemObject::isLink(const String& path)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("isLink() called for \"%1\"", path) );

				FSTreeEntryRef entry = getTreeEntry(path);

				if ( entry )
				{
					FileInformation info = entry->getFileInfo();
					if( info.type == FileInformation::E_FILE_SYMLINK )
					{
						LOG_DEBUG3( Format("isLink() returning true for \"%1\"", path) );
						return true;
					}
				}
				LOG_DEBUG3( Format("isLink() returning false for \"%1\"", path) );
				return false;
			}

			bool CannedFileSystemObject::removeFile(const String& path)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("removeFile() called for \"%1\"", path) );

				FSTreeEntryRef entry = getTreeEntry(path);

				String dir = dirname(path);
				String base = basename(path);

				if ( entry )
				{
					if( !entry->isDirectory() )
					{
						FSTreeEntryRef parentDir = findDirectory(dir);
						// getChildren() throws, so this is safe.
						parentDir->getChildren().erase(basename(path));
						LOG_DEBUG3( Format("removeFile() returning true for \"%1\"", path) );
						return true;
					}
					else
					{
						LOG_DEBUG3( Format("removeFile() Not removing directory (not implemented) \"%1\"", path) );
					}
				}
				LOG_DEBUG3( Format("removeFile() returning false for \"%1\"", path) );
				return false;
			}

			bool CannedFileSystemObject::exists(const String& path)
			{
				if ( getTreeEntry(path) )
				{
					return true;
				}
				return false;
			}

			String CannedFileSystemObject::readSymbolicLink(const String& path)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("readSymbolicLink() called for \"%1\"", path) );

				FSTreeEntryRef entry = getTreeEntry(path);

				if (!entry )
				{
					BLOCXX_THROW( blocxx::FileSystemException
						, Format("Symlink does not exist: \"%1\"", path).c_str() );
				}

				FSEntryRef fsEntry = entry->getFileEntry();

				if( !fsEntry || fsEntry->getFileType() != FileInformation::E_FILE_SYMLINK )
				{
					BLOCXX_THROW( blocxx::FileSystemException
						, Format("Failed to read symlink (not a symlink) \"%1\"", path).c_str() );
				}

				String linkContents = fsEntry->contents();
				LOG_DEBUG3( Format("readSymbolicLink() Found link \"%1\" with contents \"%2\""
						, path, linkContents) );
				return linkContents;
			}

			bool CannedFileSystemObject::canRead(const String& path)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("canRead() called for \"%1\"", path) );

				if ( isDirectory( path ) )
				{
					LOG_DEBUG3( Format("Not a file in canRead() (returning true): \"%1\"", path) );
					return true;
				}
				FSEntryRef file = getFileRef(path);

				if (!file)
				{
					LOG_DEBUG3( Format("canRead() could not find file \"%1\"", path) );
					return false;
				}

				if ( file->readable() )
				{
					LOG_DEBUG3( Format("File is readable: \"%1\"", path) );
					return true;
				}
				LOG_DEBUG3( Format("Not readable: \"%1\"", path) );
				return false;
			}

			bool CannedFileSystemObject::canWrite(const String& path)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("canWrite() called for \"%1\"", path) );

				if ( isDirectory( path ) )
				{
					LOG_DEBUG3( Format("Not a file in canWrite() (returning true): \"%1\"", path) );
					return true;
				}
				FSEntryRef file = getFileRef(path);

				if (!file)
				{
					LOG_DEBUG3( Format("canWrite() could not find file \"%1\"", path) );
					return false;
				}

				if ( file->writeable() )
				{
					LOG_DEBUG3( Format("File is writeable: \"%1\"", path) );
					return true;
				}
				LOG_DEBUG3( Format("Not writeable: \"%1\"", path) );
				return false;
			}

			String CannedFileSystemObject::getFileContents(const String& filename)
			{
				Logger logger(COMPONENT_NAME);
				return doGetFileContents(filename, 0);
			}

			String CannedFileSystemObject::doGetFileContents(const String& filename, size_t depth)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("getFileContents() called for \"%1\"", filename) );

				FSEntryRef file = getFileRef(filename);

				if (!file)
				{
					LOG_DEBUG3( Format("getFileContents() could not find file \"%1\"", filename) );
					BLOCXX_THROW( blocxx::FileSystemException
						, Format("Failed to open file \"%1\"", filename).c_str() );
				}
				LOG_DEBUG3( Format("getFileContents() using mock entry for \"%1\"", filename) );

				if (file->getFileType() == FileInformation::E_FILE_SYMLINK)
				{
					if (depth < MAX_SYMLINK_DEPTH)
					{
						LOG_DEBUG3( Format("getFileContents() following mock entry for symlink \"%1\"", filename) );
						return doGetFileContents(file->contents(), depth + 1);
					}
					else
					{
						BLOCXX_THROW( blocxx::FileSystemException
							, Format("maximum symlink depth exceeded for file \"%1\"", filename).c_str() );
					}
				}

				return file->contents();
			}

			StringArray CannedFileSystemObject::getFileLines(const String& filename)
			{
				String contents = getFileContents(filename);

				return contents.tokenize("\r\n");
			}

			bool CannedFileSystemObject::getDirectoryContents(const String& path
				, StringArray& dirEntries)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("getDirectoryContents() called for \"%1\"", path) );

				FSTreeEntryRef dirEntry = findDirectory(path);

				if (!dirEntry)
				{
					LOG_DEBUG3( Format("No directory entry for \"%1\"", path) );
					return false;
				}

				if (!dirEntry->isDirectory())
				{
					LOG_DEBUG3( Format("Entry is not a directory: \"%1\"", dirEntry->getFullPath()) );
					return false;
				}
				dirEntries.clear();

				LOG_DEBUG3(Format("Getting children of %1", dirEntry->getFullPath()));
				const FSTreeEntry::MapType& children = dirEntry->getChildren();
				for (FSTreeEntry::MapType::const_iterator fileEntry = children.begin();
					  fileEntry != children.end();
					  ++fileEntry)
				{
					dirEntries.push_back(fileEntry->first);
				}
				LOG_DEBUG3( Format("Obtained directory listing for \"%1\"", path) );
				LogUtils::dumpToLog(dirEntries, logger, "File Entry: ");
				return true;
			}
#undef LOG_DEBUG3

			FSMockObjectRef createCannedFSObject(const String& name)
			{
				return Reference<CannedFileSystemObject>(new CannedFileSystemObject(name));
			}

			bool addFSFile(FSMockObjectRef& object, FSEntryRef file)
			{
				Reference<CannedFileSystemObject> realobject = object.cast_to<CannedFileSystemObject>();

				if (!realobject)
				{
					return false;
				}
				realobject->addFile(file);
				return true;
			}

			bool addNormalFile(FSMockObjectRef& object
				, const String& path
				, const String& contents
				, PermissionFlags mode
				, UserId owner)
			{
				return CannedFSImpl::addFSFile( object
					, FSEntryRef( new NormalFile(path, contents, mode
							, FileInformation::E_FILE_REGULAR, owner) ) );
			}

			bool addNormalFile(FSMockObjectRef& object
				, const String& path
				, const String& contents
				, const FileInformation& info)
			{
				return CannedFSImpl::addFSFile( object
					, FSEntryRef( new NormalFile(path, contents, info) ) );
			}

			bool addSymlink(FSMockObjectRef& object, const String& path, const String& value, UserId owner)
			{
				return CannedFSImpl::addFSFile( object
					, FSEntryRef( new NormalFile(path, value, E_FILE_MODE_777
							, FileInformation::E_FILE_SYMLINK, owner) ) );
			}

			// FIXME! [KH-20070823] Fix the canned filesystem so directories can have
			// permissions...
			bool addDirectory(FSMockObjectRef& object, const String& path, UserId owner)
			{
				return CannedFSImpl::addFSFile( object
					, FSEntryRef( new NormalFile(path, "", E_FILE_MODE_777
							, FileInformation::E_FILE_DIRECTORY, owner) ) );
			}

		} // end namespace CannedFSImpl
	} // end namespace Test
} // end namespace BLOCXX_NAMESPACE
