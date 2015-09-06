#pragma once
#include <Windows.h>
#include "ZShadeSandboxEditor3D.h"
#include "ZShadeSandboxEditor2D.h"
#include "ZShadeSandboxInventoryEditor.h"
#include "ZShadeSandboxMenuEditor.h"
#include "ZShadeSandboxHUDEditor.h"
#include "EngineOptions.h"
#include "ZShadeResource.h"
#include "ZShadeSandboxEditorHelper.h"

namespace ZShadeSandbox {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for ZShadeSandboxEditorPicker
	/// </summary>
	public ref class ZShadeSandboxEditorPicker : public System::Windows::Forms::Form
	{
	public:
		ZShadeSandboxEditorPicker(EngineOptions* eo)
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
		~ZShadeSandboxEditorPicker()
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
	private: System::Windows::Forms::Button^  MapEditorButton;
	private: System::Windows::Forms::Button^  InventoryEditorButton;
	private: System::Windows::Forms::Button^  MenuEditorButton;
	private: System::Windows::Forms::Button^  HUDEditorButton;

		EngineOptions* mEngineOptions;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->MapEditorButton = (gcnew System::Windows::Forms::Button());
			this->InventoryEditorButton = (gcnew System::Windows::Forms::Button());
			this->MenuEditorButton = (gcnew System::Windows::Forms::Button());
			this->HUDEditorButton = (gcnew System::Windows::Forms::Button());
			this->SuspendLayout();
			// 
			// MapEditorButton
			// 
			this->MapEditorButton->Location = System::Drawing::Point(25, 25);
			this->MapEditorButton->Name = L"MapEditorButton";
			this->MapEditorButton->Size = System::Drawing::Size(75, 23);
			this->MapEditorButton->TabIndex = 1;
			this->MapEditorButton->Text = L"Map Editor";
			this->MapEditorButton->UseVisualStyleBackColor = true;
			this->MapEditorButton->Click += gcnew System::EventHandler(this, &ZShadeSandboxEditorPicker::MapEditorButton_Click);
			// 
			// InventoryEditorButton
			// 
			this->InventoryEditorButton->Location = System::Drawing::Point(110, 25);
			this->InventoryEditorButton->Name = L"InventoryEditorButton";
			this->InventoryEditorButton->Size = System::Drawing::Size(75, 23);
			this->InventoryEditorButton->TabIndex = 1;
			this->InventoryEditorButton->Text = L"Inventory Editor";
			this->InventoryEditorButton->UseVisualStyleBackColor = true;
			this->InventoryEditorButton->Click += gcnew System::EventHandler(this, &ZShadeSandboxEditorPicker::InventoryEditorButton_Click);
			// 
			// MenuEditorButton
			// 
			this->MenuEditorButton->Location = System::Drawing::Point(25, 58);
			this->MenuEditorButton->Name = L"MenuEditorButton";
			this->MenuEditorButton->Size = System::Drawing::Size(75, 23);
			this->MenuEditorButton->TabIndex = 1;
			this->MenuEditorButton->Text = L"Menu Editor";
			this->MenuEditorButton->UseVisualStyleBackColor = true;
			this->MenuEditorButton->Click += gcnew System::EventHandler(this, &ZShadeSandboxEditorPicker::MenuEditorButton_Click);
			// 
			// HUDEditorButton
			// 
			this->HUDEditorButton->Location = System::Drawing::Point(110, 58);
			this->HUDEditorButton->Name = L"HUDEditorButton";
			this->HUDEditorButton->Size = System::Drawing::Size(75, 23);
			this->HUDEditorButton->TabIndex = 1;
			this->HUDEditorButton->Text = L"HUD Editor";
			this->HUDEditorButton->UseVisualStyleBackColor = true;
			this->HUDEditorButton->Click += gcnew System::EventHandler(this, &ZShadeSandboxEditorPicker::HUDEditorButton_Click);
			// 
			// ZShadeSandboxEditorPicker
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(227, 113);
			this->ControlBox = false;
			this->Controls->Add(this->MapEditorButton);
			this->Controls->Add(this->InventoryEditorButton);
			this->Controls->Add(this->MenuEditorButton);
			this->Controls->Add(this->HUDEditorButton);
			this->Name = L"ZShadeSandboxEditorPicker";
			this->Text = L"ZShadeSandboxEditorPicker";
			this->ResumeLayout(false);

		}
#pragma endregion
		private: System::Void MapEditorButton_Click(System::Object^  sender, System::EventArgs^  e)
		{
			switch (mEngineOptions->m_DimType)
			{
				case ZSHADE_2D:
				{
					//ZShadeSandboxEditor2D^ form = gcnew ZShadeSandboxEditor2D(mEngineOptions);
					//form->ClientSize = System::Drawing::Size(mEngineOptions->m_screenWidth, mEngineOptions->m_screenHeight);
					//form->Init();
					//form->Show();
					ZShadeSandbox::ZShadeSandboxEditorHelper::Instance->zShadeSandboxEditor2D->Show();
				}
				break;
				case ZSHADE_3D:
				{
					//ZShadeSandboxEditor3D^ form = gcnew ZShadeSandboxEditor3D(mEngineOptions);
					//form->ClientSize = System::Drawing::Size(mEngineOptions->m_screenWidth, mEngineOptions->m_screenHeight);
					//form->Show();
					ZShadeSandbox::ZShadeSandboxEditorHelper::Instance->zShadeSandboxEditor3D->Show();
				}
				break;
			}
		}
	private: System::Void InventoryEditorButton_Click(System::Object^  sender, System::EventArgs^  e)
	{
		//ZShadeSandboxInventoryEditor^ form = gcnew ZShadeSandboxInventoryEditor(mEngineOptions);
		//form->ClientSize = System::Drawing::Size(mEngineOptions->m_screenWidth, mEngineOptions->m_screenHeight);
		//form->Show();
		ZShadeSandbox::ZShadeSandboxEditorHelper::Instance->zShadeSandboxInventoryEditor->Show();
	}
	private: System::Void MenuEditorButton_Click(System::Object^  sender, System::EventArgs^  e)
	{
		//ZShadeSandboxMenuEditor^ form = gcnew ZShadeSandboxMenuEditor(mEngineOptions);
		//form->ClientSize = System::Drawing::Size(mEngineOptions->m_screenWidth, mEngineOptions->m_screenHeight);
		//form->Show();
		ZShadeSandbox::ZShadeSandboxEditorHelper::Instance->zShadeSandboxMenuEditor->Show();
	}
	private: System::Void HUDEditorButton_Click(System::Object^  sender, System::EventArgs^  e)
	{
		//ZShadeSandboxHUDEditor^ form = gcnew ZShadeSandboxHUDEditor(mEngineOptions);
		//form->ClientSize = System::Drawing::Size(mEngineOptions->m_screenWidth, mEngineOptions->m_screenHeight);
		//form->Show();
		ZShadeSandbox::ZShadeSandboxEditorHelper::Instance->zShadeSandboxHUDEditor->Show();
	}
	};
}
