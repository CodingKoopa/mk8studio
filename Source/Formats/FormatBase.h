#ifndef FORMATBASE_H
#define FORMATBASE_H

#include <QString>

class FormatBase
{
  // When saving is implemented, this class will be more useful.
public:
  const QString& GetName();
  void SetName(const QString& value);

  const QString& GetPath();
  void SetPath(const QString& value);

protected:
  QString m_name;
  QString m_path;
};

#endif  // FORMATBASE_H
