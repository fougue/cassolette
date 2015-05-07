/****************************************************************************
** CharSet Tool
** Copyright Fougue Ltd. (15 Apr. 2014)
** contact@fougsys.fr
**
** This software is a computer program whose purpose is to analyse and convert
** the encoding of text files.
**
** This software is governed by the CeCILL-B license under French law and
** abiding by the rules of distribution of free software.  You can  use,
** modify and/ or redistribute the software under the terms of the CeCILL-B
** license as circulated by CEA, CNRS and INRIA at the following URL
** "http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html".
****************************************************************************/

#include <QtWidgets/QApplication>

#include "charset_tool_main_window.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // Define settings informations once
    QCoreApplication::setOrganizationName("Fougue");
    QCoreApplication::setOrganizationDomain("www.fougsys.fr");
    QCoreApplication::setApplicationName(QLatin1String("charset-tool"));
    //QCoreApplication::setApplicationVersion(versionNumber);

    CharsetToolMainWindow mainWin;
    mainWin.show();

    return app.exec();
}
