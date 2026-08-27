#pragma once

#include <vector>
#include "normativeDocumentsEditWindow.hpp"

struct NormativeDocument;

class NormativeDocumentsWindow
{
public:
    void Show(std::vector<NormativeDocument> &normativeDocumentsList);

private:
    bool normativeDocumentEditWindowIsOpen = false;
    NormativeDocumentsEditWindow normativeDocumentEditWindow;
};
