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

#include "editable_list_widget.h"

#include <fougtools/qttools/gui/item_view_buttons.h>

EditableListWidget::EditableListWidget(QWidget *parent)
    : QListWidget(parent)
{
    auto itemBtns = new qttools::ItemViewButtons(this, this);

    QIcon removeIcon;
    removeIcon.addPixmap(QPixmap(":/images/list-remove.png"), QIcon::Normal);
    removeIcon.addPixmap(QPixmap(":/images/list-remove_active.png"), QIcon::Active);

    itemBtns->installDefaultItemDelegate();
    itemBtns->addButton(0);
    itemBtns->setButtonIcon(0, removeIcon);
    itemBtns->setButtonToolTip(0, tr("Delete item"));
    itemBtns->setButtonDetection(0, -1, QVariant());
    QObject::connect(
                itemBtns, &qttools::ItemViewButtons::buttonClicked,
                this, &EditableListWidget::onItemButtonClicked);
}

void EditableListWidget::onItemButtonClicked(int btnId, const QModelIndex &index)
{
    if (btnId == 0) { // "Delete" button
        this->model()->removeRow(index.row());
    }
}
