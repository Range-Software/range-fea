#ifndef PROBLEM_SELECTOR_TILES_H
#define PROBLEM_SELECTOR_TILES_H

#include <QListWidget>

class ProblemSelectorTree;

//! Problem types shown as tiles with an icon and a name.
//! Selection is kept in given ProblemSelectorTree, which also applies the rules for required and excluded problem types.
class ProblemSelectorTiles : public QListWidget
{

    Q_OBJECT

    protected:

        //! Problem selector tree holding the selection.
        ProblemSelectorTree *problemTree;

    public:

        //! Constructor.
        explicit ProblemSelectorTiles(ProblemSelectorTree *problemTree, QWidget *parent = nullptr);

        //! Return recommended size, which fits all tiles.
        QSize sizeHint() const override;

    protected:

        //! Handle key press event.
        void keyPressEvent(QKeyEvent *event) override;

        //! Toggle selection of given tile.
        void toggleItem(QListWidgetItem *item);

    protected slots:

        //! Update tiles from selection in problem selector tree.
        void onProblemTreeChanged();

    signals:

        //! Tile was double-clicked, which selects it and confirms the selection.
        void confirmed();

};

#endif // PROBLEM_SELECTOR_TILES_H
