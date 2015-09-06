#pragma once
#include "EngineOptions.h"

namespace ZShadeSandbox {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for ZShadeSandboxInventoryEditor
	/// </summary>
	public ref class ZShadeSandboxInventoryEditor : public System::Windows::Forms::Form
	{
	public:
		ZShadeSandboxInventoryEditor(EngineOptions* eo)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
			mEngineOptions = eo;
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~ZShadeSandboxInventoryEditor()
		{
			if (components)
			{
				delete components;
			}
		}

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

		EngineOptions* mEngineOptions;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = gcnew System::ComponentModel::Container();
			this->Size = System::Drawing::Size(300,300);
			this->Text = L"ZShadeSandboxInventoryEditor";
			this->Padding = System::Windows::Forms::Padding(0);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
		}
#pragma endregion
	};
}
