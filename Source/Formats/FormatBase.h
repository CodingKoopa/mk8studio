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
protected:
  QString m_name;
};

#endif  // FORMATBASE_H
