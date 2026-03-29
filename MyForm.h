#pragma once
#include "compressor.h"
#include <msclr/marshal_cppstd.h>
#include <sstream>

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace msclr::interop;

namespace mainwinapp {

    public ref class MyForm : public System::Windows::Forms::Form {
    public:
        MyForm(void) {
            InitializeComponent();
        }

    protected:
        ~MyForm() {
            if (components) {
                delete components;
            }
        }

    private:
        Button^ compressButton;
        Button^ decompressButton;
        GroupBox^ statsPanel;
        TextBox^ statsTextBox;
        System::ComponentModel::Container^ components;

#pragma region Windows Form Designer generated code
        void InitializeComponent(void) {
            this->compressButton = (gcnew System::Windows::Forms::Button());
            this->decompressButton = (gcnew System::Windows::Forms::Button());
            this->statsPanel = (gcnew System::Windows::Forms::GroupBox());
            this->statsTextBox = (gcnew System::Windows::Forms::TextBox());
            this->statsPanel->SuspendLayout();
            this->SuspendLayout();

            // 
            // compressButton
            // 
            this->compressButton->Location = System::Drawing::Point(30, 30);
            this->compressButton->Name = L"compressButton";
            this->compressButton->Size = System::Drawing::Size(100, 40);
            this->compressButton->TabIndex = 0;
            this->compressButton->Text = L"Compress";
            this->compressButton->UseVisualStyleBackColor = true;
            this->compressButton->Click += gcnew System::EventHandler(this, &MyForm::compressButton_Click);
            // 
            // decompressButton
            // 
            this->decompressButton->Location = System::Drawing::Point(150, 30);
            this->decompressButton->Name = L"decompressButton";
            this->decompressButton->Size = System::Drawing::Size(100, 40);
            this->decompressButton->TabIndex = 1;
            this->decompressButton->Text = L"Decompress";
            this->decompressButton->UseVisualStyleBackColor = true;
            this->decompressButton->Click += gcnew System::EventHandler(this, &MyForm::decompressButton_Click);
            // 
            // statsPanel
            // 
            this->statsPanel->Controls->Add(this->statsTextBox);
            this->statsPanel->Location = System::Drawing::Point(30, 90);
            this->statsPanel->Name = L"statsPanel";
            this->statsPanel->Size = System::Drawing::Size(400, 150);
            this->statsPanel->TabIndex = 2;
            this->statsPanel->TabStop = false;
            this->statsPanel->Text = L"Compression Statistics";
            // 
            // statsTextBox
            // 
            this->statsTextBox->Location = System::Drawing::Point(10, 25);
            this->statsTextBox->Multiline = true;
            this->statsTextBox->Name = L"statsTextBox";
            this->statsTextBox->ReadOnly = true;
            this->statsTextBox->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
            this->statsTextBox->Size = System::Drawing::Size(380, 110);
            this->statsTextBox->TabIndex = 0;
            // 
            // MyForm
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->ClientSize = System::Drawing::Size(470, 270);
            this->Controls->Add(this->statsPanel);
            this->Controls->Add(this->decompressButton);
            this->Controls->Add(this->compressButton);
            this->Name = L"MyForm";
            this->Text = L"Data Squeeze";
            this->statsPanel->ResumeLayout(false);
            this->statsPanel->PerformLayout();
            this->ResumeLayout(false);
        }
#pragma endregion

    private:
        void compressButton_Click(System::Object^ sender, System::EventArgs^ e) {
            OpenFileDialog^ openFileDialog = gcnew OpenFileDialog();
            openFileDialog->Filter = "Text Files (*.txt)|*.txt|All Files (*.*)|*.*";
            if (openFileDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
                String^ inputPath = openFileDialog->FileName;
                SaveFileDialog^ saveFileDialog = gcnew SaveFileDialog();
                saveFileDialog->Filter = "Compressed Files (*.bin)|*.bin|All Files (*.*)|*.*";
                if (saveFileDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
                    String^ outputPath = saveFileDialog->FileName;

                    std::string inputFile = marshal_as<std::string>(inputPath);
                    std::string outputFile = marshal_as<std::string>(outputPath);

                    TextCompressor compressor;
                    compressor.compressFile(inputFile, outputFile);

                    std::string stats = compressor.getStatisticsString();

                    if (!stats.empty()) {
                        statsTextBox->Text = gcnew String(stats.c_str());
                    }
                    else {
                        statsTextBox->Text = "Compression failed!";
                    }
                }
            }
        }


        void decompressButton_Click(System::Object^ sender, System::EventArgs^ e) {
            OpenFileDialog^ openFileDialog = gcnew OpenFileDialog();
            openFileDialog->Filter = "Compressed Files (*.bin)|*.bin|All Files (*.*)|*.*";
            if (openFileDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
                String^ inputPath = openFileDialog->FileName;
                SaveFileDialog^ saveFileDialog = gcnew SaveFileDialog();
                saveFileDialog->Filter = "Text Files (*.txt)|*.txt|All Files (*.*)|*.*";
                if (saveFileDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
                    std::string inputFile = marshal_as<std::string>(inputPath);
                    std::string outputFile = marshal_as<std::string>(saveFileDialog->FileName);

                    TextCompressor compressor;
                    bool success = compressor.decompressFile(inputFile, outputFile);

                    if (success) {
                        statsTextBox->Text = "Decompression completed.";
                    }
                    else {
                        statsTextBox->Text = "Decompression failed!";
                    }
                }
            }
        }
    };
}
