#pragma once

#include <QString>

class FormatBase
{
  // When saving is implemented, this class will be more useful.
public:
  const QString& GetName() const;
  void SetName(const QString& value);

  const QString& GetPath() const;
  void SetPath(const QString& value);

protected:
  QString m_name;
  QString m_path;
};
