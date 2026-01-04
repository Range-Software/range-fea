#ifndef GL_STATE_CACHE_H
#define GL_STATE_CACHE_H

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <QtOpenGL>

//! Singleton class that caches OpenGL state to avoid expensive glGet*() queries.
//! Instead of querying the GPU for current state, we track state changes in software.
class GLStateCache
{

    protected:

        //! Cached boolean states.
        GLboolean lineSmooth;
        GLboolean lighting;
        GLboolean normalize;
        GLboolean depthTest;
        GLboolean blend;
        GLboolean texture1D;
        GLboolean texture2D;
        GLboolean cullFace;

        //! Cached integer states.
        GLenum cullFaceMode;

        //! Cached float states.
        GLfloat pointSize;
        GLfloat lineWidth;

        //! Whether the cache has been initialized.
        bool initialized;

    private:

        //! Private constructor for singleton.
        GLStateCache();

    public:

        //! Get singleton instance.
        static GLStateCache &instance();

        //! Initialize cache by reading current OpenGL state (call once per context).
        void initialize();

        //! Reset cache (call when context changes).
        void reset();

        //! Check if cache is initialized.
        bool isInitialized() const;

        // Boolean state getters (no GPU query)
        GLboolean getLineSmooth() const;
        GLboolean getLighting() const;
        GLboolean getNormalize() const;
        GLboolean getDepthTest() const;
        GLboolean getBlend() const;
        GLboolean getTexture1D() const;
        GLboolean getTexture2D() const;
        GLboolean getCullFace() const;

        // Integer state getters (no GPU query)
        GLenum getCullFaceMode() const;

        // Float state getters (no GPU query)
        GLfloat getPointSize() const;
        GLfloat getLineWidth() const;

        // State setters (updates cache and makes GL call only if changed)
        void setLineSmooth(GLboolean enabled);
        void setLighting(GLboolean enabled);
        void setNormalize(GLboolean enabled);
        void setDepthTest(GLboolean enabled);
        void setBlend(GLboolean enabled);
        void setTexture1D(GLboolean enabled);
        void setTexture2D(GLboolean enabled);
        void setCullFace(GLboolean enabled);
        void setCullFaceMode(GLenum mode);
        void setPointSize(GLfloat size);
        void setLineWidth(GLfloat width);

        // Convenience methods for enable/disable
        void enableLineSmooth();
        void disableLineSmooth();
        void enableLighting();
        void disableLighting();
        void enableNormalize();
        void disableNormalize();
        void enableDepthTest();
        void disableDepthTest();
        void enableBlend();
        void disableBlend();
        void enableTexture1D();
        void disableTexture1D();
        void enableTexture2D();
        void disableTexture2D();
        void enableCullFace();
        void disableCullFace();

};

#endif // GL_STATE_CACHE_H
