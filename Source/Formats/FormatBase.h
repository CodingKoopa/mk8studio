#ifndef FORMATBASE_H
#define FORMATBASE_H

#include <QString>

class FormatBase
{
  // When saving is implemented, this class will be more useful.
public:
  FormatBase() {}
  QString GetName() { return m_name; }
  void SetName(QString value) { m_name = value; }

  QString GetPath() { return m_path; }
  void SetPath(QString value) { m_path = value; }

protected:
  QString m_name;
  QString m_path;
};

#endif  // FORMATBASE_H
