#pragma once
#include <Windows.h>
//#include "ZShadeSandboxEditor2D.h"
//#include "ZShadeSandboxEditor3D.h"
//#include "ZShadeSandboxInventoryEditor.h"
//#include "ZShadeSandboxMenuEditor.h"
//#include "ZShadeSandboxHUDEditor.h"
#include "EngineOptions.h"
#include "EditorHelper2D.h"
#include "ZShadeResource.h"
#include "Environment.h"
#include "Environment2D.h"
#include "Environment3D.h"

// For the DXGameWindow I could do a Shutdown on the D3D class
// and then do an initialize with new window handle each time
// a new editor is chosen.

namespace ZShadeSandbox {
	ref class ZShadeSandboxEditor2D;
	ref class ZShadeSandboxEditor3D;
	ref class ZShadeSandboxInventoryEditor;
	ref class ZShadeSandboxMenuEditor;
	ref class ZShadeSandboxHUDEditor;
}

namespace ZShadeSandbox {
	ref class ZShadeSandboxEditorHelper
	{
	private:

		ZShadeSandboxEditorHelper()
		{
			bDXWindowInitialized = false;
			mEngineOptions = 0;
			mEditorHelper2D = 0;
			mDXGameWindow = 0;
		}

		ZShadeSandboxEditorHelper(const ZShadeSandboxEditorHelper%)
		{
			throw gcnew System::InvalidOperationException("singleton cannot be copy-constructed");
		}

		static ZShadeSandboxEditorHelper mInstance;

		ZShadeSandbox::ZShadeSandboxEditor2D^ mZShadeSandboxEditor2D;
		ZShadeSandbox::ZShadeSandboxEditor3D^ mZShadeSandboxEditor3D;
		ZShadeSandbox::ZShadeSandboxInventoryEditor^ mZShadeSandboxInventoryEditor;
		ZShadeSandbox::ZShadeSandboxMenuEditor^ mZShadeSandboxMenuEditor;
		ZShadeSandbox::ZShadeSandboxHUDEditor^ mZShadeSandboxHUDEditor;

		DXGameWindow* mDXGameWindow;
		EditorHelper2D* mEditorHelper2D;
		EngineOptions* mEngineOptions;
		bool bDXWindowInitialized;

	public:

		property bool DXWindowInitialized
		{
			bool get() { return bDXWindowInitialized; }
			void set(bool value) { bDXWindowInitialized = value; }
		}

		property DXGameWindow* dxGameWindow
		{
			DXGameWindow* get() { return mDXGameWindow; }
			void set(DXGameWindow* value) { mDXGameWindow = value; }
		}

		property EditorHelper2D* editorHelper2D
		{
			EditorHelper2D* get() { return mEditorHelper2D; }
			void set(EditorHelper2D* value) { mEditorHelper2D = value; }
		}

		property EngineOptions* engineOptions
		{
			EngineOptions* get() { return mEngineOptions; }
			void set(EngineOptions* value) { mEngineOptions = value; }
		}

		static property ZShadeSandboxEditorHelper^ Instance
		{
			ZShadeSandboxEditorHelper^ get() { return %mInstance; }
		}

		property ZShadeSandbox::ZShadeSandboxEditor2D^ zShadeSandboxEditor2D
		{
			ZShadeSandbox::ZShadeSandboxEditor2D^ get() { return mZShadeSandboxEditor2D; }
			void set(ZShadeSandbox::ZShadeSandboxEditor2D^ value) { mZShadeSandboxEditor2D = value; }
		}
		property ZShadeSandbox::ZShadeSandboxEditor3D^ zShadeSandboxEditor3D
		{
			ZShadeSandbox::ZShadeSandboxEditor3D^ get() { return mZShadeSandboxEditor3D; }
			void set(ZShadeSandbox::ZShadeSandboxEditor3D^ value) { mZShadeSandboxEditor3D = value; }
		}
		property ZShadeSandbox::ZShadeSandboxInventoryEditor^ zShadeSandboxInventoryEditor
		{
			ZShadeSandbox::ZShadeSandboxInventoryEditor^ get() { return mZShadeSandboxInventoryEditor; }
			void set(ZShadeSandbox::ZShadeSandboxInventoryEditor^ value) { mZShadeSandboxInventoryEditor = value; }
		}
		property ZShadeSandbox::ZShadeSandboxMenuEditor^ zShadeSandboxMenuEditor
		{
			ZShadeSandbox::ZShadeSandboxMenuEditor^ get() { return mZShadeSandboxMenuEditor; }
			void set(ZShadeSandbox::ZShadeSandboxMenuEditor^ value) { mZShadeSandboxMenuEditor = value; }
		}
		property ZShadeSandbox::ZShadeSandboxHUDEditor^ zShadeSandboxHUDEditor
		{
			ZShadeSandbox::ZShadeSandboxHUDEditor^ get() { return mZShadeSandboxHUDEditor; }
			void set(ZShadeSandbox::ZShadeSandboxHUDEditor^ value) { mZShadeSandboxHUDEditor = value; }
		}

	public:

		// This is ran in the game window thread
		void RunDXWindowInThreadStart(bool& iniContainsGameFolder, bool& initLoadCache, HWND toolHwnd)
		{
			if (mDXGameWindow)
			{
				if (!mEditorHelper2D)
				{
					mEditorHelper2D = new EditorHelper2D();
					mEditorHelper2D->ToolHwnd() = toolHwnd;

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
					mEditorHelper2D->ToolHwnd() = toolHwnd;

					// Has already been loaded but might be switching to a different editor
					// that might need to load a cache
					if (iniContainsGameFolder)
					{
						iniContainsGameFolder = false;
						initLoadCache = true;
					}
				}
			}
		}
		void RunDXWindowInThread()
		{
			while (mDXGameWindow && !mDXGameWindow->IsDestroyed())
			{
				// When EditorHelper3D is created then do a dim check on which game directory is created
				if (EditorHelper2D::GameDirectoryCreated())
				{
					mDXGameWindow->Run();
				}
			}
		}

		void InitEditorWindows();

		void InitDXWindow(HWND hwnd)
		{
			if (!bDXWindowInitialized)
			{
				mDXGameWindow = new DXGameWindow();
				mDXGameWindow->Init(mEngineOptions, hwnd);
				bDXWindowInitialized = true;
			}
		}

		void SwitchD3DWindow(HWND hwnd)
		{
			// The environment only exists if the DXGameWindow has been created
			if (bDXWindowInitialized && mDXGameWindow)
			{
				ZShadeSandboxEnvironment::Environment* env = DXGameWindow::Env();

				switch (mEngineOptions->m_DimType)
				{
				case ZSHADE_2D:
				{
								  Environment2D* env2D = env->Env2D();
								  env2D->SwitchD3DWindow(hwnd);
				}
					break;
				case ZSHADE_3D:
				{
								  Environment3D* env3D = env->Env3D();
								  env3D->SwitchD3DWindow(hwnd);
				}
					break;
				}
			}
		}
	};
}

/*#pragma once
#pragma managed(push, on)
#include <Windows.h>

#include "ZShadeSandboxEditor2D.h"
#include "ZShadeSandboxEditor3D.h"
#include "ZShadeSandboxInventoryEditor.h"
#include "ZShadeSandboxMenuEditor.h"
#include "ZShadeSandboxHUDEditor.h"

#include "EngineOptions.h"
#include "EditorHelper2D.h"
#include "ZShadeResource.h"

//namespace ZShadeSandbox {
//	ref class ZShadeSandboxEditor2D;
//	ref class ZShadeSandboxEditor3D;
//	ref class ZShadeSandboxInventoryEditor;
//	ref class ZShadeSandboxMenuEditor;
//	ref class ZShadeSandboxHUDEditor;
//}

namespace ZShadeSandbox {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Threading;

	/// <summary>
	/// Summary for ZShadeSandboxHelper
	/// </summary>
	public ref class ZShadeSandboxHelper
	{
	public:
		ZShadeSandboxHelper()
		{
		}
		
		static std::string ConvertToStandardString(System::String^ string)
		{
			using System::Runtime::InteropServices::Marshal;
			System::IntPtr pointer = Marshal::StringToHGlobalAnsi(string);
			char* charPointer = reinterpret_cast<char*>(pointer.ToPointer());
			std::string returnString(charPointer, string->Length);
			Marshal::FreeHGlobal(pointer);
			return returnString;
		}
		
		static void SetDXGameWindow(DXGameWindow* dxWin) { mDXGameWindow = dxWin; }
		static DXGameWindow* GetDXGameWindow() { return mDXGameWindow; }
		
		static void SetEditorHelper2D(EditorHelper2D* eh2D) { mEditorHelper2D = eh2D; }
		static EditorHelper2D* GetEditorHelper2D() { return mEditorHelper2D; }
		
		//static void SetEditorHelper3D(EditorHelper3D* eh3D) { mEditorHelper3D = eh3D; }
		//static EditorHelper3D* GetEditorHelper3D() { return mEditorHelper3D; }
		
		static void SetEngineOptions(EngineOptions* eo) { mEngineOptions = eo; }
		static EngineOptions* GetEngineOptions() { return mEngineOptions; }
		
		static void SetInitialized(bool value) { bInitialized = value; }
		static bool IsInitialized() { return bInitialized; }
		
		static ZShadeSandboxHelper^ Instance();
		
		/*static ZShadeSandboxEditor2D^ zShadeSandboxEditor2D() { return mZShadeSandboxEditor2D; }
		static ZShadeSandboxEditor3D^ zShadeSandboxEditor3D() { return mZShadeSandboxEditor3D; }
		static ZShadeSandboxInventoryEditor^ zShadeSandboxInventoryEditor() { return mZShadeSandboxInventoryEditor; }
		static ZShadeSandboxMenuEditor^ zShadeSandboxMenuEditor() { return mZShadeSandboxMenuEditor; }
		static ZShadeSandboxHUDEditor^ zShadeSandboxHUDEditor() { return mZShadeSandboxHUDEditor; }*
		
		static void InitEditorWindows();
		
		// This is ran in the game window thread
		static void RunDXWindowInThread(bool& iniContainsGameFolder, bool& initLoadCache);
		
		static void InitDXWindow(HWND hwnd);
		
	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~ZShadeSandboxHelper()
		{
		}
	protected:

	private:
		
		static ZShadeSandboxHelper^ instance;
		
		/*static ZShadeSandboxEditor2D^ mZShadeSandboxEditor2D;
		static ZShadeSandboxEditor3D^ mZShadeSandboxEditor3D;
		static ZShadeSandboxInventoryEditor^ mZShadeSandboxInventoryEditor;
		static ZShadeSandboxMenuEditor^ mZShadeSandboxMenuEditor;
		static ZShadeSandboxHUDEditor^ mZShadeSandboxHUDEditor;*
		
		static EngineOptions* mEngineOptions;
		static DXGameWindow* mDXGameWindow;
		static EditorHelper2D* mEditorHelper2D;
		// static EditorHelper3D* mEditorHelper3D;
		
		static bool bInitialized;
	};
}
#pragma managed(pop)
*/