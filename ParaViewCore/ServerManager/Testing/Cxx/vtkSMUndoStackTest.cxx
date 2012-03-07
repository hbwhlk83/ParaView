/*=========================================================================

Program:   ParaView
Module:    vtkSMUndoStackTest.cxx

Copyright (c) Kitware, Inc.
All rights reserved.
See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkSMUndoStackTest.h"

#include "vtkInitializationHelper.h"
#include "vtkPVServerOptions.h"
#include "vtkProcessModule.h"
#include "vtkSMUndoStack.h"
#include "vtkUndoSet.h"
#include "vtkSMSession.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkSMPropertyModificationUndoElement.h"
#include "vtkSMProxy.h"
#include "vtkSMRemoteObjectUpdateUndoElement.h"
#include "vtkSMMessage.h"

void vtkSMUndoStackTest::UndoRedo()
{
  vtkSMSession *session = vtkSMSession::New();
  vtkSMSessionProxyManager *pxm = session->GetSessionProxyManager();

  vtkSMProxy *sphere = pxm->NewProxy("sources", "SphereSource");
  sphere->UpdateVTKObjects();
  QVERIFY(sphere != NULL);
  QCOMPARE(vtkSMPropertyHelper(sphere, "Radius").GetAsDouble(), 0.5);

  vtkSMUndoStack *undoStack = vtkSMUndoStack::New();
  vtkUndoSet *undoSet = vtkUndoSet::New();
  vtkSMRemoteObjectUpdateUndoElement *undoElement = vtkSMRemoteObjectUpdateUndoElement::New();
  undoElement->SetSession(session);

  vtkSMMessage before;
  before.CopyFrom(*sphere->GetFullState());
  vtkSMPropertyHelper(sphere, "Radius").Set(1.2);
  sphere->UpdateVTKObjects();
  vtkSMMessage after;
  after.CopyFrom(*sphere->GetFullState());
  undoElement->SetUndoRedoState(&before, &after);

  undoSet->AddElement(undoElement);
  undoElement->Delete();
  undoStack->Push("ChangeRadius", undoSet);
  undoSet->Delete();

  QVERIFY(undoStack->CanUndo() == true);
  undoStack->Undo();
  QVERIFY(undoStack->CanUndo() == false);
  sphere->UpdateVTKObjects();
  QCOMPARE(vtkSMPropertyHelper(sphere, "Radius").GetAsDouble(), 0.5);

  QVERIFY(undoStack->CanRedo() == true);
  undoStack->Redo();
  sphere->UpdateVTKObjects();
  QCOMPARE(vtkSMPropertyHelper(sphere, "Radius").GetAsDouble(), 1.2);
  QVERIFY(undoStack->CanRedo() == false);

  undoStack->Delete();

  sphere->Delete();
  session->Delete();
}

void vtkSMUndoStackTest::StackDepth()
{
  vtkSMUndoStack *stack = vtkSMUndoStack::New();
  QCOMPARE(stack->GetStackDepth(), 10);
  stack->Delete();
}

int main(int argc, char *argv[])
{
  vtkPVServerOptions* options = vtkPVServerOptions::New();
  vtkInitializationHelper::Initialize(argc, argv,
                                      vtkProcessModule::PROCESS_CLIENT,
                                      options);

  vtkSMUndoStackTest test;
  int ret = QTest::qExec(&test, argc, argv);

  vtkInitializationHelper::Finalize();
  options->Delete();

  return ret;
}
