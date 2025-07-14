#ifndef GL_LIST_H
#define GL_LIST_H

#include <vector>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <QOpenGLFunctions>

class GLList
{

    protected:

        //! OpenGL list base.
        GLuint listBase;
        //! Number og OpenGL lists.
        GLuint nLists;
        //! Array of booleans indicating whether the list is valid.
        std::vector<bool> listValid;

    private:

        //! Internal initialization function.
        void _init (const GLList *pGlList = nullptr);

    public:

        //! Constructor.
        GLList(GLuint nLists = 0);

        //! Copy constructor.
        GLList(const GLList &glList);

        //! Destructor.
        ~GLList();

        //! Assignment operator.
        GLList & operator = (const GLList &glList);

        //! Return list base.
        GLuint getBase() const;

        //! Return number of lists.
        GLuint getNLists() const;

        //! Set new number of lists.
        //! If different than the previous, then
        //! old lists will be deleted and new generated.
        void setNLists(GLuint nLists);

        //! Return whether the list at given position is valid.
        bool getListValid(GLuint listPosition) const;

        //! Set list at given position as in/valid.
        void setListInvalid(GLuint listPosition);

        //! Return whether the lists are valid.
        bool getValid() const;

        //! Start creating new list - start compile.
        void newList(GLuint listPosition, GLenum mode = GL_COMPILE);

        //! Stop creating new list - end compile.
        void endList(GLuint listPosition);

        //! Call created and valid list at given possition.
        void callList(GLuint listPosition) const;

        //! Call all created and valid lists.
        void callLists() const;

};

#endif /* GL_LIST_H */
