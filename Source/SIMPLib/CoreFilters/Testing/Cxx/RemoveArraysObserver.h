#ifndef _removearraysobserver_h_
#define _removearraysobserver_h_

#include <QtCore/QList>
#include <QtCore/QObject>

#include "SIMPLib/Common/PipelineMessage.h"
#include "SIMPLib/Common/SIMPLibSetGetMacros.h"
#include "SIMPLib/SIMPLib.h"
#include "SIMPLib/SIMPLibVersion.h"

class RemoveArraysObserver : public QObject
{
  Q_OBJECT

  SIMPL_INSTANCE_PROPERTY(QList<PipelineMessage>, ErrorList)
  Q_PROPERTY(QList<PipelineMessage> ErrorList READ getErrorList WRITE setErrorList)

public:
  SIMPL_SHARED_POINTERS(RemoveArraysObserver)
  SIMPL_TYPE_MACRO(RemoveArraysObserver)
  SIMPL_STATIC_NEW_MACRO(RemoveArraysObserver)

  RemoveArraysObserver()
  {
  }
  virtual ~RemoveArraysObserver()
  {
  }

public slots:
  void processPipelineMessage(const PipelineMessage& msg)
  {
    m_ErrorList.push_back(msg);
  }

private:
  RemoveArraysObserver(const RemoveArraysObserver&) = delete; // Copy Constructor Not Implemented
  void operator=(const RemoveArraysObserver&);                // Move assignment Not Implemented
};

#endif /* _RemoveArraysObserver_H_ */
