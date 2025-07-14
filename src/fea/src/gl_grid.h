#ifndef GL_GRID_H
#define GL_GRID_H

#include "gl_object.h"

class GLGrid : public GLObject
{

    private:

        // Environment settings.

    protected:

        double scale;
        RLimitBox limitBox;
        double gMin;
        double gMax;
        double gdt;

    private:

        //! Internal initialization function.
        void _init(const GLGrid *pGlGrid = nullptr);

    public:

        //! Constructor.
        explicit GLGrid(GLWidget *glWidget,
                        double scale,
                        const RLimitBox &limitBox);

        //! Copy constructor.
        GLGrid(const GLGrid &glGrid);

        //! Destructor.
        virtual ~GLGrid() {}

        //! Assignment operator.
        GLGrid &operator=(const GLGrid &glGrid);

    protected:

        //! Initialize scene.
        void initialize();

        //! Finalize scene.
        void finalize();

        //! Draw scene.
        void draw();

};

#endif // GL_GRID_H
