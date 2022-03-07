#pragma once

#include <vector>
#include <memory>
#include <utils/CicadaType.h>

class VideoFormatPrivate;
/*!
 * \brief The VideoFormat class
 * Describes the layout of video data. Some properties like display aspect ratio, color space and color range, which describes how to display the video frame, should be in VideoFrame class.
+ */
class CICADA_CPLUS_EXTERN VideoFormat {
public:
    /*!
     * \brief The PixelFormat enum
     * 32 bit rgba format enum name indicates it's channel layout. For example,
     * Format_ARGB32 byte layout is AARRGGBB, it's integer value is 0xAARRGGBB on big endian platforms
     * and 0xBBGGRRAA on little endian platforms
     * Format_RGB32 and QImage::Format_ARGB32 are the same.
     * TODO: 0RGB, XRGB, not native endia use R8 or R16. ffmpeg does not have native endian format
     * currently 0rgb xrgb use rgba formats and check hasAlpha() is required
     */
    enum PixelFormat {
        Format_Invalid = -1,
        Format_ARGB32, // AARRGGBB or 00RRGGBB, check hasAlpha is required
        Format_BGRA32, //BBGGRRAA
        Format_ABGR32, // QImage.RGBA8888 le
        Format_RGBA32, // QImage. no
        Format_RGB32, // 0xAARRGGBB native endian. same as QImage::Format_ARGB32. be: ARGB32, le: BGRA32
        Format_BGR32, // 0xAABBGGRR native endian
        Format_RGB24,
        Format_BGR24,
        Format_RGB565,
        Format_BGR565,
        Format_RGB555,
        Format_BGR555,

        //http://www.fourcc.org/yuv.php
        Format_AYUV444,
        Format_YUV444P,
        Format_YUV422P,
        Format_YUV420P,
        Format_YUV411P,
        Format_YUV410P,
        Format_YV12,
        Format_UYVY, //422
        Format_VYUY, //not in ffmpeg. OMX_COLOR_FormatCrYCbY
        Format_YUYV, //422, aka yuy2
        Format_YVYU, //422
        Format_NV12,
        Format_NV21,
        Format_IMC1,
        Format_IMC2,
        Format_IMC3, //same as IMC1, swap U V
        Format_IMC4, //same as IMC2, swap U V
        Format_Y8, //GREY. single 8 bit Y plane
        Format_Y16, //single 16 bit Y plane. LE

        Format_Jpeg, //yuvj

        //Format_CameraRaw,
        //Format_AdobeDng,

        Format_YUV420P9LE,
        Format_YUV422P9LE,
        Format_YUV444P9LE,
        Format_YUV420P10LE,
        Format_YUV422P10LE,
        Format_YUV444P10LE,
        Format_YUV420P12LE,
        Format_YUV422P12LE,
        Format_YUV444P12LE,
        Format_YUV420P14LE,
        Format_YUV422P14LE,
        Format_YUV444P14LE,
        Format_YUV420P16LE,
        Format_YUV422P16LE,
        Format_YUV444P16LE,
        Format_YUV420P9BE,
        Format_YUV422P9BE,
        Format_YUV444P9BE,
        Format_YUV420P10BE,
        Format_YUV422P10BE,
        Format_YUV444P10BE,
        Format_YUV420P12BE,
        Format_YUV422P12BE,
        Format_YUV444P12BE,
        Format_YUV420P14BE,
        Format_YUV422P14BE,
        Format_YUV444P14BE,
        Format_YUV420P16BE,
        Format_YUV422P16BE,
        Format_YUV444P16BE,

        Format_RGB48, // native endian
        Format_RGB48LE,
        Format_RGB48BE,
        Format_BGR48,
        Format_BGR48LE,
        Format_BGR48BE,
        Format_RGBA64, //native endian
        Format_RGBA64LE,
        Format_RGBA64BE,
        Format_BGRA64, //native endian
        Format_BGRA64LE,
        Format_BGRA64BE,

        Format_VYU, // for rgb422_apple texture, the layout is like rgb24: (v, y, u, )
        Format_XYZ12,
        Format_XYZ12LE,
        Format_XYZ12BE,
        Format_User
    };

    static PixelFormat pixelFormatFromFFmpeg(int ff); //AVPixelFormat
    static int pixelFormatToFFmpeg(PixelFormat fmt);
    static std::vector<int> pixelFormatsFFmpeg();

    VideoFormat(PixelFormat format = Format_Invalid);
    VideoFormat(int formatFF);
    VideoFormat(const std::string& name);
    VideoFormat(const VideoFormat &other);
    ~VideoFormat();

    VideoFormat& operator=(const VideoFormat &other);
    VideoFormat& operator=(VideoFormat::PixelFormat pixfmt);
    VideoFormat& operator=(int ffpixfmt);
    bool operator==(const VideoFormat &other) const;
    bool operator==(VideoFormat::PixelFormat pixfmt) const;
    bool operator==(int ffpixfmt) const;
    bool operator!=(const VideoFormat &other) const;
    bool operator!=(VideoFormat::PixelFormat pixfmt) const;
    bool operator!=(int ffpixfmt) const;

    bool isValid() const;

    PixelFormat pixelFormat() const;
    int pixelFormatFFmpeg() const;
    std::string name() const;
    /*!
     * \brief setPixelFormat set pixel format to format. other information like bpp will be updated
     * \param format
     */
    void setPixelFormat(PixelFormat format);
    void setPixelFormatFFmpeg(int format);

    /*!
     * \brief channels
     * \return number of channels(components) the the format. e.g. RGBA has 4 channels, NV12 is 3
     */
    int channels() const;
    /*!
     * \brief channels
     * \param plane
     * \return number of channels in a plane
     */
    int channels(int plane) const;
    /*!
     * \brief planeCount
     * \return -1 if not a valid format
     */
    int planeCount() const;
    /*!
     * https://wiki.videolan.org/YUV
     * bytesPerPixel()
     *  YUV420P: 1pix = 4Y+U+V, (4*8+8+8)/4 = 12
     * bytesPerPixel(plane) is different, for example
     * uyvy422 bytesPerPixel(0) = 8+8+8 = 24, while bytesPerPixel() = (2*8+8+8)/2 = 16
     */
    int bitsPerPixel() const;
    /// nv12: 16 for uv plane
    int bitsPerPixel(int plane) const;
    /// bgr24 is 24 not 32
    int bitsPerPixelPadded() const;
    int bytesPerPixel() const;
    int bytesPerPixel(int plane) const;
    /*!
     * \brief bitsPerComponent
     * \return number of bits per component (0 if uneven)
     */
    int bitsPerComponent() const;

    // return line size with given width
    int bytesPerLine(int width, int plane) const;
    /*!
     * \brief chromaWidth
     * \param lumaWidth
     * \return U, V component (or channel) width for the given luma width.
     */
    int chromaWidth(int lumaWidth) const;
    int chromaHeight(int lumaHeight) const;
    /*!
     * \brief width
     * plane width for given lumaWidth in current format
     * \return lumaWidth if plane <= 0. otherwise chromaWidth
     */
    int width(int lumaWidth, int plane) const;
    int height(int lumaHeight, int plane) const;
    /*!
     * \brief normalizedWidth
     * \return 1.0 for plane <= 0. otherwise chroma width
     */
    double normalizedWidth(int plane) const;
    double normalizedHeight(int plane) const;
    //TODO: add planeWidth()/planeHeight()
    // test AV_PIX_FMT_FLAG_XXX
    bool isBigEndian() const;
    bool hasPalette() const;
    bool isPseudoPaletted() const;
    /**
     * All values of a component are bit-wise packed end to end.
     */
    bool isBitStream() const;
    /**
     * Pixel format is an HW accelerated format.
     */
    bool isHWAccelerated() const;
    /*!
     * \brief isPlanar
     * \return true if is planar or semi planar
     *
     * Semi-planar: 2 planes instead of 3, one plane for luminance, and one plane for both chrominance components.
     * They are also sometimes referred to as biplanar formats also
     * Packed: 1 plane
     * Planar: 1 plane for each component (channel)
     */
    bool isPlanar() const;
    bool isRGB() const;
    bool isXYZ() const;
    bool hasAlpha() const;

    static bool isPlanar(PixelFormat pixfmt);
    static bool isRGB(PixelFormat pixfmt);
    static bool hasAlpha(PixelFormat pixfmt);

private:
    std::unique_ptr<VideoFormatPrivate> d;
};
