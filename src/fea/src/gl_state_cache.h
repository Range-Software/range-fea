#ifndef GL_STATE_CACHE_H
#define GL_STATE_CACHE_H

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <QtOpenGL>

// Forward declaration — avoids pulling in gl_shader_program.h everywhere.
class GLShaderProgram;

//! Singleton class that caches rendering state to avoid expensive glGet*() queries.
//! Instead of querying the GPU for current state, we track state changes in software.
//!
//! The cache is the single source of truth for both rendering backends: with the
//! legacy OpenGL backend every setter also issues the matching GL call, with the
//! QRhi backend no GL call is made and the cached values are used to pick the
//! graphics pipeline for each draw call.
class GLStateCache
{

    protected:

        //! Cached boolean states.
        GLboolean lineSmooth;
        GLboolean lighting;
        GLboolean normalize;
        GLboolean depthTest;
        GLboolean depthMask;
        GLboolean blend;
        GLboolean texture1D;
        GLboolean texture2D;
        GLboolean cullFace;
        GLboolean lineStipple;
        GLboolean polygonOffsetFill;

        //! Cached integer states.
        GLenum cullFaceMode;
        GLenum depthFunc;

        //! Cached float states.
        GLfloat pointSize;
        GLfloat lineWidth;
        GLfloat polygonOffsetFactor;
        GLfloat polygonOffsetUnits;

        //! Whether back faces are shaded like front faces (cut planes, iso surfaces).
        bool twoSided;

        //! Whether the clipping plane is active.
        bool clipPlaneEnabled;
        //! Clipping plane equation in eye space.
        double clipPlane[4];

        //! Whether the cache has been initialized.
        bool initialized;

        //! Currently bound shader program (nullptr when fixed-function pipeline active).
        //! When non-null, setLighting() also updates the uUseLighting uniform.
        GLShaderProgram *shaderProgram;

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
        GLboolean getDepthMask() const;
        GLboolean getBlend() const;
        GLboolean getTexture1D() const;
        GLboolean getTexture2D() const;
        GLboolean getCullFace() const;
        GLboolean getLineStipple() const;
        GLboolean getPolygonOffsetFill() const;

        // Integer state getters (no GPU query)
        GLenum getCullFaceMode() const;
        GLenum getDepthFunc() const;

        // Float state getters (no GPU query)
        GLfloat getPointSize() const;
        GLfloat getLineWidth() const;
        GLfloat getPolygonOffsetFactor() const;
        GLfloat getPolygonOffsetUnits() const;

        //! Return whether back faces are shaded like front faces.
        bool getTwoSided() const;

        //! Return whether the clipping plane is active.
        bool getClipPlaneEnabled() const;

        //! Return the clipping plane equation in eye space.
        const double *getClipPlane() const;

        //! Register the currently bound shader so setLighting() can sync uUseLighting.
        //! Pass nullptr when the shader is released (fixed-function rendering active).
        void setShaderProgram(GLShaderProgram *prog);

        //! Return the currently registered shader program.
        GLShaderProgram *getShaderProgram() const;

        // State setters (updates cache and makes GL call only if changed)
        void setLineSmooth(GLboolean enabled);
        void setLighting(GLboolean enabled);
        void setNormalize(GLboolean enabled);
        void setDepthTest(GLboolean enabled);
        void setDepthMask(GLboolean enabled);
        void setBlend(GLboolean enabled);
        void setTexture1D(GLboolean enabled);
        void setTexture2D(GLboolean enabled);
        void setCullFace(GLboolean enabled);
        void setCullFaceMode(GLenum mode);
        void setDepthFunc(GLenum func);
        void setLineStipple(GLboolean enabled, GLint factor = 6, GLushort pattern = 0xAAAA);
        void setPolygonOffsetFill(GLboolean enabled);
        void setPolygonOffset(GLfloat factor, GLfloat units);
        void setPointSize(GLfloat size);
        void setLineWidth(GLfloat width);
        void setTwoSided(bool twoSided);
        void setClipPlane(bool enabled, const double plane[4] = nullptr);

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
