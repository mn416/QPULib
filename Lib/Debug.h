#ifndef DEBUG_H
#define DEBUG_H
#include <string>
#include "Common/Seq.h"
#include "Target/Syntax.h"

class Stmt;


/**
 * @brief Handles the outputting of debug information.
 *
 * Allows dynamic setting of the debug output within a
 * program, without having to resort to recompiling.
 *
 * In addition, allows debug output to file.
 */
class Debug
{
public:
  Debug();
  ~Debug();

  static FILE *getFile();
  static void enable(bool enabled, bool toFile = false, const std::string &filename = "");

  static void emitSourceCode(Stmt *body);
  static void emitTargetCode(Seq<Instr> *targetCode);
  static void emitMapmem(unsigned base, void *mem);
  static void emitMboxProperty(void *buf);

private:
  static const char *DEFAULT_FILE;

  bool        m_enabled;    // true if debug info should be outputted
  bool        m_toFile;     // if true, output to file, otherwise to stdout
  std::string m_filename;
  FILE       *m_file;

  static Debug m_instance;  // Singleton for this class

  FILE *getFileIntern();
  std::string getFilename();
  void enableIntern(bool enabled, bool toFile, const std::string &filename);
  void openFile(const std::string filename);
  void closeFile();
};

#endif // DEBUG_H
