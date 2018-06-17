#include "Debug.h"
#include <cassert>
#include <cstdio>
#include "Source/Pretty.h"


const char *Debug::DEFAULT_FILE = "debug.log";  // The default filename to use for file output
Debug Debug::m_instance;                        // Singleton for this class


Debug::Debug() :
  m_enabled(false),
  m_toFile(false),
  m_filename(DEFAULT_FILE),
  m_file(nullptr)
{
}


Debug::~Debug()
{
  closeFile();
}


/**
 * @brief Set the debug output options
 *
 * @param enabled  if true, output debug information
 * @param toFile   if true, output to file, otherwise output to stdout
 * @param filename if set, name of file to output to
 */
void Debug::enable(bool enabled, bool toFile, const std::string &filename)
{
  m_instance.enable_intern(enabled, toFile, filename);
}


void Debug::enable_intern(bool enabled, bool toFile, const std::string &filename)
{
  if (enabled && toFile)
  {
    openFile(filename);
  }
  else
  {
    closeFile();
  }

  m_enabled = enabled;
  m_toFile  = toFile;
  m_filename = filename;  // Note that name gets saved always, might be used later on if
                          // debug output enabled.
}


void Debug::emitSourceCode(Stmt *body)
{
  FILE *f = m_instance.getFile();
  if (f == nullptr) return;

  fprintf(f, "Source code\n");
  fprintf(f, "===========\n\n");
  pretty(f, body);
  fprintf(f, "\n");
  fflush(f);
}


/**
 * @brief Determine and return the file pointer for use as output
 *
 * @return null if output not enabled, otherwise a valid file pointer;
 *         can also be stdout.
 */
FILE *Debug::getFile()
{
  if (!m_enabled) return nullptr;
  if (!m_toFile) return stdout;
  return m_file;
}


/**
 * @brief Determine current filename to use
 */
std::string Debug::getFilename()
{
  if (!m_filename.empty()) return m_filename;  // Use current filename if present
  return DEFAULT_FILE;
}


/**
 * @brief Open a file for debug output
 *
 * If a file was already open, it will be reopened if the
 * filename changed.
 */
void Debug::openFile(const std::string filename) {
  // determine filename to use
  std::string tmpFilename = filename;
  if (tmpFilename.empty()) tmpFilename = m_instance.getFilename();

  bool nameChanged = (m_instance.getFilename() != tmpFilename);

  // Need to reopen any open file if name changed
  if (m_file != nullptr && nameChanged)
  {
    m_instance.closeFile();
  }

  assert(m_file == nullptr);

  if (m_file == nullptr)
  {
    // open file if not already done so
    assert(!tmpFilename.empty());
    FILE *tmpFile = fopen(tmpFilename.c_str(), "w");
    assert(tmpFile != nullptr);
    if (tmpFile != nullptr)
    {
      m_file = tmpFile;
    }
    else
    {
      printf("ERROR: could not open file '%s' for debug output\n", m_filename.c_str());
    }
  }
}


/**
 * @brief Close off an open file, if any
 */
void Debug::closeFile() {
  if (m_file != nullptr)
  {
    int result = fclose(m_file);
    assert(result == 0);  // Value 0 means success
    m_file = nullptr;
  }
}
