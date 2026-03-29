#include <msclr/marshal_cppstd.h>
#include "MyForm.h"

using namespace msclr::interop;
using namespace System;
using namespace System::Windows::Forms;

[STAThreadAttribute]
int main(array<String^>^ args) {
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);
    Application::Run(gcnew mainwinapp::MyForm());
    return 0;
}
