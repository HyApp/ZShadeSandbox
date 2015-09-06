#include "ZShadeSandboxEditorHelper.h"
#include "ZShadeSandboxEditor2D.h"
#include "ZShadeSandboxEditor3D.h"
#include "ZShadeSandboxInventoryEditor.h"
#include "ZShadeSandboxMenuEditor.h"
#include "ZShadeSandboxHUDEditor.h"

void ZShadeSandbox::ZShadeSandboxEditorHelper::InitEditorWindows()
{
	mZShadeSandboxEditor2D = (gcnew ZShadeSandbox::ZShadeSandboxEditor2D(mEngineOptions));
	mZShadeSandboxEditor2D->ClientSize = System::Drawing::Size(mEngineOptions->m_screenWidth, mEngineOptions->m_screenHeight);
	mZShadeSandboxEditor2D->Init();

	mZShadeSandboxEditor3D = (gcnew ZShadeSandbox::ZShadeSandboxEditor3D(mEngineOptions));
	mZShadeSandboxEditor3D->ClientSize = System::Drawing::Size(mEngineOptions->m_screenWidth, mEngineOptions->m_screenHeight);
	//mZShadeSandboxEditor2D->Init();

	mZShadeSandboxInventoryEditor = (gcnew ZShadeSandbox::ZShadeSandboxInventoryEditor(mEngineOptions));
	mZShadeSandboxInventoryEditor->ClientSize = System::Drawing::Size(mEngineOptions->m_screenWidth, mEngineOptions->m_screenHeight);
	//mZShadeSandboxInventoryEditor->Init();

	mZShadeSandboxMenuEditor = (gcnew ZShadeSandbox::ZShadeSandboxMenuEditor(mEngineOptions));
	mZShadeSandboxMenuEditor->ClientSize = System::Drawing::Size(mEngineOptions->m_screenWidth, mEngineOptions->m_screenHeight);
	//mZShadeSandboxMenuEditor->Init();

	mZShadeSandboxHUDEditor = (gcnew ZShadeSandbox::ZShadeSandboxHUDEditor(mEngineOptions));
	mZShadeSandboxHUDEditor->ClientSize = System::Drawing::Size(mEngineOptions->m_screenWidth, mEngineOptions->m_screenHeight);
	//mZShadeSandboxHUDEditor->Init();
}
/*#include "Environment.h"
#include "ZShadeSandboxEditor2D.h"
#include "ZShadeSandboxEditor3D.h"
#include "ZShadeSandboxInventoryEditor.h"
#include "ZShadeSandboxMenuEditor.h"
#include "ZShadeSandboxHUDEditor.h"

ZShadeSandbox::ZShadeSandboxHelper* ZShadeSandbox::ZShadeSandboxHelper::instance = 0;
EngineOptions* ZShadeSandbox::ZShadeSandboxHelper::mEngineOptions = 0;
EditorHelper2D* ZShadeSandbox::ZShadeSandboxHelper::mEditorHelper2D = 0;
DXGameWindow* ZShadeSandbox::ZShadeSandboxHelper::mDXGameWindow = 0;
bool ZShadeSandbox::ZShadeSandboxHelper::bInitialized = false;

ZShadeSandbox::ZShadeSandboxHelper^ ZShadeSandbox::ZShadeSandboxHelper::Instance()
{
	if (instance == 0)
	{
		instance = (gcnew ZShadeSandbox::ZShadeSandboxHelper());
	}
	
	return instance;
}

void ZShadeSandbox::ZShadeSandboxHelper::InitEditorWindows()
{
	mZShadeSandboxEditor2D = (gcnew ZShadeSandbox::ZShadeSandboxEditor2D(mEngineOptions));
	mZShadeSandboxEditor2D->ClientSize = System::Drawing::Size(mEngineOptions->m_screenWidth, mEngineOptions->m_screenHeight);
	mZShadeSandboxEditor2D->Init();
	
	mZShadeSandboxEditor3D = (gcnew ZShadeSandbox::ZShadeSandboxEditor3D(mEngineOptions));
	mZShadeSandboxEditor3D->ClientSize = System::Drawing::Size(mEngineOptions->m_screenWidth, mEngineOptions->m_screenHeight);
	//mZShadeSandboxEditor2D->Init();
	
	mZShadeSandboxInventoryEditor = (gcnew ZShadeSandbox::ZShadeSandboxInventoryEditor(mEngineOptions));
	mZShadeSandboxInventoryEditor->ClientSize = System::Drawing::Size(mEngineOptions->m_screenWidth, mEngineOptions->m_screenHeight);
	//mZShadeSandboxInventoryEditor->Init();
	
	mZShadeSandboxMenuEditor = (gcnew ZShadeSandbox::ZShadeSandboxMenuEditor(mEngineOptions));
	mZShadeSandboxMenuEditor->ClientSize = System::Drawing::Size(mEngineOptions->m_screenWidth, mEngineOptions->m_screenHeight);
	//mZShadeSandboxMenuEditor->Init();
	
	mZShadeSandboxHUDEditor = (gcnew ZShadeSandbox::ZShadeSandboxHUDEditor(mEngineOptions));
	mZShadeSandboxHUDEditor->ClientSize = System::Drawing::Size(mEngineOptions->m_screenWidth, mEngineOptions->m_screenHeight);
	//mZShadeSandboxHUDEditor->Init();
}

void ZShadeSandbox::ZShadeSandboxHelper::RunDXWindowInThread(bool& iniContainsGameFolder, bool& initLoadCache)
{
	if (mDXGameWindow)
	{
		if (!mEditorHelper2D)
		{
			mEditorHelper2D = new EditorHelper2D();
			mEditorHelper2D->ToolHwnd() = mToolHwnd;
			
			if (iniContainsGameFolder)
			{
				mEditorHelper2D->LoadINIGameDirectory();
				iniContainsGameFolder = false;
				initLoadCache = true;
			}
			
			ZShadeSandboxEnvironment::Environment::Env2D()->SetMapEditorSystem(mEditorHelper2D->GetMapEditorSystem2D());
			ZShadeSandboxEnvironment::Environment::Env2D()->SetMenuEditorSystem(mEditorHelper2D->GetMenuEditorSystem());
			ZShadeSandboxEnvironment::Environment::Env2D()->SetHUDEditorSystem(mEditorHelper2D->GetHUDEditorSystem());
			ZShadeSandboxEnvironment::Environment::Env2D()->SetInventoryEditorSystem2D(mEditorHelper2D->GetInventoryEditorSystem2D());
			mEditorHelper2D->GetMapEditorSystem2D()->SwitchToEditModeTiles();
		}
		else
		{
			// Has already been loaded but might be switching to a different editor
			// that might need to load a cache
			if (iniContainsGameFolder)
			{
				iniContainsGameFolder = false;
				initLoadCache = true;
			}
		}
	}
	
	while (mDXGameWindow && !mDXGameWindow->IsDestroyed())
	{
		if (EditorHelper2D::GameDirectoryCreated())
		{
			mDXGameWindow->Run();
		}
	}
}

void ZShadeSandbox::ZShadeSandboxHelper::InitDXWindow(HWND hwnd)
{
	if (!bInitialized)
	{
		mDXGameWindow = new DXGameWindow();
		mDXGameWindow->Init(mEngineOptions, hwnd);
		bInitialized = true;
	}
}*/