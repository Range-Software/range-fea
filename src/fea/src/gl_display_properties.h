#ifndef GL_DISPLAY_PROPERTIES_H
#define GL_DISPLAY_PROPERTIES_H

#include <QString>

#include <rml_gl_display_properties.h>

class GLDisplayProperties : public RGLDisplayProperties
{

    protected:

        //! File name.
        QString fileName;

    private:

        //! Internal initialization function.
        void _init(const GLDisplayProperties *pDisplayProperties = nullptr);

    public:

        //! Constructor.
        GLDisplayProperties();

        //! Copy constructor.
        GLDisplayProperties(const GLDisplayProperties &displayProperties);

        //! Destructor.
        ~GLDisplayProperties();

        //! Assignment operator.
        GLDisplayProperties & operator =(const GLDisplayProperties &displayProperties);

        //! Return const reference to file name.
        const QString &getFileName() const;

        //! Set file name.
        void setFileName(const QString &fileName);

        //! Load display prperties.
        void load(const QString &fileName);

        //! Store display properties.
        void store() const;

};

#endif // GL_DISPLAY_PROPERTIES_H
