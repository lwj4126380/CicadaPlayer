#pragma once
#include "OpenGLHelper.h"
#include "OpenGLTypes.h"
#include "base/media/IAFPacket.h"
#include "base/media/VideoFormat.h"
#include "platform/platform_gl.h"
#include "qrect.h"
#include "qvector2d.h"
#include <map>
#include <memory>
#include <vector>

class VideoMaterial;
class VideoShaderPrivate;
/*!
 * \brief The VideoShader class
 * Represents a shader for rendering a video frame.
 * Low-level api. Used by OpenGLVideo and Scene Graph.
 * You can also create your own shader. Usually only sampling function and rgb post processing are enough.
 * Transforming color to rgb is done internally.
 * TODO: vertex shader (fully controlled by user?). Mesh
 */
class VideoShader {
public:
    VideoShader();
    virtual ~VideoShader();
    /*!
     * \brief attributeNames
     * Array must end with null. { position, texcoord, ..., 0}, location is bound to 0, 1, ...
     * \return
     */
    virtual char const *const *attributeNames() const;
    /*!
     * \brief vertexShader
     * mvp uniform: u_Matrix
     * Vertex shader in: a_Position, a_TexCoordsN (see attributeNames())
     * Vertex shader out: v_TexCoordsN
     */
    virtual const char *vertexShader() const;
    virtual const char *fragmentShader() const;
    /*!
     * \brief initialize
     * \param shaderProgram: 0 means create a shader program internally. if not linked, vertex/fragment shader will be added and linked
     */
    virtual void initialize();
    int uniformLocation(const char *name) const;
    /*!
     * \brief textureLocationCount
     * number of texture locations is
     * 1: packed RGB
     * number of channels: yuv or plannar RGB
     */
    int textureLocationCount() const;
    int textureLocation(int index) const;
    int matrixLocation() const;
    int colorMatrixLocation() const;
    int opacityLocation() const;
    int channelMapLocation() const;
    int texelSizeLocation() const;
    int textureSizeLocation() const;
    // defalut is GL_TEXTURE_2D
    int textureTarget() const;
    /*!
     * \brief update
     * Upload textures, setup uniforms before rendering.
     * If material type changed, build a new shader program.
     */
    bool update(VideoMaterial *material);

protected:
    /// rebuild shader program before next rendering. call this if shader code is updated
    void rebuildLater();

private:
    /*!
     * \brief programReady
     * Called when program is linked and all uniforms are resolved
     */
    virtual void programReady()
    {}

    /// User configurable shader APIs BEGIN
    /*!
     * Keywords will be replaced in user shader code:
     * %planes% => plane count
     * Uniforms can be used: (N: 0 ~ planes-1)
     * u_Matrix (vertex shader),
     * u_TextureN, v_TexCoordsN, u_texelSize(array of vec2, normalized), u_textureSize(array of vec2), u_opacity, u_c(channel map), u_colorMatrix, u_to8(vec2, computing 16bit value with 8bit components)
     * Vertex shader in: a_Position, a_TexCoordsN (see attributeNames())
     * Vertex shader out: v_TexCoordsN
     */
    /*!
     * \brief userShaderHeader
     * Must add additional uniform declarations here
     */
    virtual const char *userShaderHeader(OpenGLHelper::ShaderType) const
    {
        return 0;
    }
    /*!
     * \brief setUserUniformValues
     * Call program()->setUniformValue(...) here
     * You can upload a texture for blending in userPostProcess(),
     * or LUT texture used by userSample() or userPostProcess() etc.
     * \return false if use use setUserUniformValue(Uniform& u), true if call program()->setUniformValue() here
     */
    virtual bool setUserUniformValues()
    {
        return false;
    }
    /*!
     * \brief setUserUniformValue
     * Update value of uniform u. Call Uniform.set(const T& value, int count); VideoShader will call Uniform.setGL() later if value is changed
     */
    virtual void setUserUniformValue(Uniform &)
    {}
    /*!
     * \brief userSample
     * Fragment shader only. The custom sampling function to replace texture2D()/texture() (replace %1 in shader).
     * \code
     *     vec4 sample2d(sampler2D tex, vec2 pos, int plane) { .... }
     * \endcode
     * The 3rd parameter can be used to get texel/texture size of a given plane u_texelSize[plane]/textureSize[plane];
     * Convolution of result rgb and kernel has the same effect as convolution of input yuv and kernel, ensured by
     * Σ_i c_i* Σ_j k_j*x_j=Σ_i k_i* Σ_j c_j*x_j
     * Because because the input yuv is from a real rgb color, so no clamp() is required for the transformed color.
     */
    virtual const char *userSample() const
    {
        return 0;
    }
    /*!
     * \brief userPostProcess
     * Fragment shader only. Process rgb color
     * TODO: parameter ShaderType?
     */
    virtual const char *userPostProcess() const
    {
        return 0;
    }
    /// User configurable shader APIs END

    bool build();
    void setVideoFormat(const VideoFormat &format);
    void setTextureTarget(int type);
    void setMaterialType(int value);
    friend class VideoMaterial;

protected:
    VideoShaderPrivate *d = nullptr;
};

class VideoMaterialPrivate;
/*!
 * \brief The VideoMaterial class
 * Encapsulates rendering state for a video shader program.
 * Low-level api. Used by OpenGLVideo and Scene Graph
 */
class VideoMaterial {
public:
    VideoMaterial();
    ~VideoMaterial();
    void setCurrentFrame(std::unique_ptr<IAFFrame> &frame);
    VideoFormat currentFormat() const;
    VideoShader *createShader() const;
    virtual int type() const;
    static std::string typeName(int value);

    bool bind();// TODO: roi
    void unbind();
    int compare(const VideoMaterial *other) const;

    int textureTarget() const;
    /*!
     * \brief isDirty
     * \return true if material type changed, or other properties changed, e.g. 8bit=>10bit (the same material type) and eq
     */
    bool isDirty() const;
    /*!
     * \brief setDirty
     * Call it after frame is rendered, i.e. after VideoShader::update(VideoMaterial*)
     */
    void setDirty(bool value);
    const QMatrix4x4 &colorMatrix() const;
    const QMatrix4x4 &channelMap() const;
    int bitsPerComponent() const;//0 if the value of components are different
    QVector2D vectorTo8bit() const;
    int planeCount() const;
    /*!
     * \brief validTextureWidth
     * Value is (0, 1]. Normalized valid width of a plane.
     * A plane may has padding invalid data at the end for aligment.
     * Use this value to reduce texture coordinate computation.
     * ---------------------
     * |                |   |
     * | valid width    |   |
     * |                |   |
     * ----------------------
     * | <- aligned width ->|
     * \return valid width ratio
     */
    double validTextureWidth() const;
    QSize frameSize() const;
    /*!
     * \brief texelSize
     * The size of texture unit
     * \return (1.0/textureWidth, 1.0/textureHeight)
     */
    QSizeF texelSize(int plane) const;//vec2?
    /*!
     * \brief texelSize
     * For GLSL. 1 for rectangle texture, 1/(width, height) for 2d texture
     */
    std::vector<QVector2D> texelSize() const;
    /*!
     * \brief textureSize
     * It can be used with a uniform to emulate GLSL textureSize() which exists in new versions.
     */
    QSize textureSize(int plane) const;
    /*!
     * \brief textureSize
     * For GLSL. Not normalized
     */
    std::vector<QVector2D> textureSize() const;
    /*!
     * \brief normalizedROI
     * \param roi logical roi of a video frame.
     * the same as mapToTexture(roi, 1)
     */
    QRectF normalizedROI(const QRectF &roi) const;
    /*!
     * \brief mapToTexture
     * map a point p or a rect r to video texture in a given plane and scaled to valid width.
     * p or r is in video frame's rect coordinates, no matter which plane is
     * \param normalize -1: auto(do not normalize for rectangle texture). 0: no. 1: yes
     * \return
     * point or rect in current texture valid coordinates. \sa validTextureWidth()
     */
    QPointF mapToTexture(int plane, const QPointF &p, int normalize = -1) const;
    QRectF mapToTexture(int plane, const QRectF &r, int normalize = -1) const;
    double brightness() const;
    void setBrightness(double value);
    double contrast() const;
    void setContrast(double value);
    double hue() const;
    void setHue(double value);
    double saturation() const;
    void setSaturation(double value);

protected:
    VideoMaterialPrivate *d = nullptr;
};

class ShaderManager {
public:
    VideoShader *prepareMaterial(VideoMaterial *material, int materialType = -1);
    ~ShaderManager();

private:
    std::map<int, VideoShader *> shader_cache;
};