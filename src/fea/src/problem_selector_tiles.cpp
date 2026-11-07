#include <algorithm>

#include <QKeyEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScrollBar>
#include <QStyledItemDelegate>

#include <rgl_icon.h>
#include <rml_problem.h>

#include "problem_selector_tiles.h"
#include "problem_selector_tree.h"

namespace
{

constexpr int tileIconSize = 48;
constexpr int tileMinimumWidth = 136;
constexpr int tileMargin = 4;
constexpr int tilePadding = 10;
constexpr int tileIconTextSpacing = 8;
constexpr int tileColumns = 4;

//! Flags used to draw name of a problem type.
constexpr int tileTextFlags = Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap;

//! Return icon file name for given problem type.
QString findProblemTypeIconFile(RProblemType problemType)
{
    switch (problemType)
    {
        case R_PROBLEM_ACOUSTICS:      return QStringLiteral(":/icons/problem/pixmaps/range-problem_type_acoustics.svg");
        case R_PROBLEM_FLUID_PARTICLE: return QStringLiteral(":/icons/problem/pixmaps/range-problem_type_fluid_particle.svg");
        case R_PROBLEM_ELECTROSTATICS: return QStringLiteral(":/icons/problem/pixmaps/range-problem_type_electrostatics.svg");
        case R_PROBLEM_MAGNETOSTATICS: return QStringLiteral(":/icons/problem/pixmaps/range-problem_type_magnetostatics.svg");
        case R_PROBLEM_FLUID:          return QStringLiteral(":/icons/problem/pixmaps/range-problem_type_fluid.svg");
        case R_PROBLEM_FLUID_HEAT:     return QStringLiteral(":/icons/problem/pixmaps/range-problem_type_fluid_heat.svg");
        case R_PROBLEM_HEAT:           return QStringLiteral(":/icons/problem/pixmaps/range-problem_type_heat.svg");
        case R_PROBLEM_RADIATIVE_HEAT: return QStringLiteral(":/icons/problem/pixmaps/range-problem_type_radiative_heat.svg");
        case R_PROBLEM_STRESS:         return QStringLiteral(":/icons/problem/pixmaps/range-problem_type_stress.svg");
        case R_PROBLEM_STRESS_MODAL:   return QStringLiteral(":/icons/problem/pixmaps/range-problem_type_stress_modal.svg");
        case R_PROBLEM_POTENTIAL:      return QStringLiteral(":/icons/problem/pixmaps/range-problem_type_potential.svg");
        case R_PROBLEM_MESH:           return QStringLiteral(":/icons/problem/pixmaps/range-problem_type_mesh.svg");
        default:                       return QStringLiteral(":/icons/problem/pixmaps/range-problem_task_flow.svg");
    }
}

//! Draws a tile: a card with the icon, name under it and a check mark in the corner.
class ProblemTileDelegate : public QStyledItemDelegate
{

    public:

        using QStyledItemDelegate::QStyledItemDelegate;

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
        {
            const bool checked = (index.data(Qt::CheckStateRole).toInt() == Qt::Checked);
            const bool hovered = option.state.testFlag(QStyle::State_MouseOver);
            const bool keyboardFocus = option.state.testFlag(QStyle::State_HasFocus) && option.state.testFlag(QStyle::State_KeyboardFocusChange);

            const QColor accentColor(option.palette.color(QPalette::Active,QPalette::Highlight));
            const QColor textColor(option.palette.color(QPalette::Active,QPalette::Text));

            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);

            // Card.
            const QRectF cardRect = QRectF(option.rect).adjusted(tileMargin+0.5,tileMargin+0.5,-tileMargin-0.5,-tileMargin-0.5);
            QColor fillColor(Qt::transparent);
            QColor borderColor(textColor);
            borderColor.setAlphaF(hovered ? 0.45f : 0.2f);
            if (checked)
            {
                fillColor = accentColor;
                fillColor.setAlphaF(0.22f);
                borderColor = accentColor;
            }
            else if (hovered)
            {
                fillColor = textColor;
                fillColor.setAlphaF(0.06f);
            }
            painter->setBrush(fillColor);
            painter->setPen(QPen(keyboardFocus ? accentColor : borderColor,(checked || keyboardFocus) ? 2.0 : 1.0));
            painter->drawRoundedRect(cardRect,8.0,8.0);

            // Icon.
            const QRect contentRect = option.rect.adjusted(tileMargin+tilePadding,tileMargin+tilePadding,-tileMargin-tilePadding,-tileMargin-tilePadding);
            const QRect iconRect(contentRect.center().x()-tileIconSize/2+1,contentRect.top(),tileIconSize,tileIconSize);
            index.data(Qt::DecorationRole).value<QIcon>().paint(painter,iconRect);

            // Name.
            painter->setPen(textColor);
            painter->setFont(option.font);
            painter->drawText(contentRect.adjusted(0,tileIconSize+tileIconTextSpacing,0,0),tileTextFlags,index.data(Qt::DisplayRole).toString());

            // Check mark.
            const qreal markRadius = 8.0;
            const QPointF markCenter(cardRect.right()-markRadius-5.0,cardRect.top()+markRadius+5.0);
            if (checked)
            {
                painter->setPen(Qt::NoPen);
                painter->setBrush(accentColor);
                painter->drawEllipse(markCenter,markRadius,markRadius);

                QPainterPath checkPath;
                checkPath.moveTo(markCenter+QPointF(-3.75,0.25));
                checkPath.lineTo(markCenter+QPointF(-1.0,3.0));
                checkPath.lineTo(markCenter+QPointF(4.0,-2.75));
                painter->setBrush(Qt::NoBrush);
                painter->setPen(QPen(option.palette.color(QPalette::Active,QPalette::HighlightedText),1.75,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
                painter->drawPath(checkPath);
            }
            else
            {
                QColor markColor(textColor);
                markColor.setAlphaF(hovered ? 0.6f : 0.3f);
                painter->setBrush(Qt::NoBrush);
                painter->setPen(QPen(markColor,1.25));
                painter->drawEllipse(markCenter,markRadius-0.5,markRadius-0.5);
            }

            painter->restore();
        }

};

} // namespace

ProblemSelectorTiles::ProblemSelectorTiles(ProblemSelectorTree *problemTree, QWidget *parent)
    : QListWidget(parent)
    , problemTree(problemTree)
{
    this->setViewMode(QListView::IconMode);
    this->setResizeMode(QListView::Adjust);
    this->setMovement(QListView::Static);
    this->setSelectionMode(QAbstractItemView::NoSelection);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->setMouseTracking(true);
    this->viewport()->setAttribute(Qt::WA_Hover);
    this->setItemDelegate(new ProblemTileDelegate(this));

    const std::vector<RProblemType> problemTypes(this->problemTree->getProblemTypes());

    // Widest word of a name has to fit on a line and all names have to fit into a tile.
    int textWidth = tileMinimumWidth - 2*(tileMargin+tilePadding);
    for (RProblemType problemType : problemTypes)
    {
        const QStringList words(RProblem::getName(problemType).split(' '));
        for (const QString &word : words)
        {
            textWidth = std::max(textWidth,this->fontMetrics().horizontalAdvance(word));
        }
    }
    int textHeight = 0;
    for (RProblemType problemType : problemTypes)
    {
        textHeight = std::max(textHeight,this->fontMetrics().boundingRect(QRect(0,0,textWidth,10000),tileTextFlags,RProblem::getName(problemType)).height());
    }
    const int inset = 2*(tileMargin+tilePadding);
    this->setGridSize(QSize(textWidth+inset,tileIconSize+tileIconTextSpacing+textHeight+inset));

    for (RProblemType problemType : problemTypes)
    {
        QListWidgetItem *item = new QListWidgetItem(RIcon::fromFile(findProblemTypeIconFile(problemType)),RProblem::getName(problemType),this);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        item->setData(Qt::UserRole,QVariant(int(problemType)));
        item->setToolTip(RProblem::getDesc(problemType));
        item->setSizeHint(this->gridSize());
    }
    this->onProblemTreeChanged();

    QObject::connect(this,&QListWidget::itemClicked,this,&ProblemSelectorTiles::toggleItem);
    QObject::connect(this->problemTree,&ProblemSelectorTree::changed,this,&ProblemSelectorTiles::onProblemTreeChanged);
}

QSize ProblemSelectorTiles::sizeHint() const
{
    const int nRows = (this->count() + tileColumns - 1) / tileColumns;
    // View wraps a tile which ends exactly at its right edge, so few extra pixels are needed.
    const int frame = 2*this->frameWidth() + 2;
    // Room for scroll bar keeps all columns when it shows up during layout, otherwise wrapped tiles would keep it shown.
    const int scrollBarWidth = this->verticalScrollBar()->sizeHint().width();
    return QSize(tileColumns*this->gridSize().width()+frame+scrollBarWidth,nRows*this->gridSize().height()+frame);
}

void ProblemSelectorTiles::keyPressEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Space || event->key() == Qt::Key_Select) && this->currentItem())
    {
        this->toggleItem(this->currentItem());
        return;
    }
    this->QListWidget::keyPressEvent(event);
}

void ProblemSelectorTiles::toggleItem(QListWidgetItem *item)
{
    this->problemTree->setProblemTypeChecked(RProblemType(item->data(Qt::UserRole).toInt()),item->checkState() != Qt::Checked);
}

void ProblemSelectorTiles::onProblemTreeChanged()
{
    for (int i=0;i<this->count();i++)
    {
        QListWidgetItem *item = this->item(i);
        item->setCheckState(this->problemTree->isProblemTypeChecked(RProblemType(item->data(Qt::UserRole).toInt())) ? Qt::Checked : Qt::Unchecked);
    }
}
