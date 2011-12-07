
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef SkGpuDevice_DEFINED
#define SkGpuDevice_DEFINED

#include "SkGr.h"
#include "SkBitmap.h"
#include "SkDevice.h"
#include "SkRegion.h"
#include "GrContext.h"

struct SkDrawProcs;
struct GrSkDrawProcs;
class GrTextContext;

/**
 *  Subclass of SkDevice, which directs all drawing to the GrGpu owned by the
 *  canvas.
 */
class SK_API SkGpuDevice : public SkDevice {
public:
    /**
     *  New device that will create an offscreen renderTarget based on the
     *  config, width, height.
     *
     *  usage is a special flag that should only be set by SkCanvas
     *  internally.
     */
    SkGpuDevice(GrContext*, SkBitmap::Config,
                int width, int height, 
                SkDevice::Usage usage = SkDevice::kGeneral_Usage);

    /**
     *  New device that will render to the specified renderTarget.
     */
    SkGpuDevice(GrContext*, GrRenderTarget*);

    /**
     *  New device that will render to the texture (as a rendertarget).
     *  The GrTexture's asRenderTarget() must be non-NULL or device will not
     *  function.
     */
    SkGpuDevice(GrContext*, GrTexture*);

    virtual ~SkGpuDevice();

    GrContext* context() const { return fContext; }

    /**
     *  Override from SkGpuDevice, so we can set our FBO to be the render target
     *  The canvas parameter must be a SkGpuCanvas
     */
    virtual void gainFocus(SkCanvas*, const SkMatrix&, const SkRegion&,
                           const SkClipStack& clipStack) SK_OVERRIDE;

    virtual SkGpuRenderTarget* accessRenderTarget() SK_OVERRIDE;

    // overrides from SkDevice

    virtual void clear(SkColor color) SK_OVERRIDE;
    virtual void writePixels(const SkBitmap& bitmap, int x, int y,
                             SkCanvas::Config8888 config8888) SK_OVERRIDE;

    virtual void setMatrixClip(const SkMatrix& matrix, const SkRegion& clip,
                               const SkClipStack&) SK_OVERRIDE;

    virtual void drawPaint(const SkDraw&, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPoints(const SkDraw&, SkCanvas::PointMode mode, size_t count,
                            const SkPoint[], const SkPaint& paint) SK_OVERRIDE;
    virtual void drawRect(const SkDraw&, const SkRect& r,
                          const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPath(const SkDraw&, const SkPath& path,
                          const SkPaint& paint, const SkMatrix* prePathMatrix,
                          bool pathIsMutable) SK_OVERRIDE;
    virtual void drawBitmap(const SkDraw&, const SkBitmap& bitmap,
                            const SkIRect* srcRectOrNull,
                            const SkMatrix&, const SkPaint&) SK_OVERRIDE;
    virtual void drawSprite(const SkDraw&, const SkBitmap& bitmap,
                            int x, int y, const SkPaint& paint);
    virtual void drawText(const SkDraw&, const void* text, size_t len,
                          SkScalar x, SkScalar y, const SkPaint&) SK_OVERRIDE;
    virtual void drawPosText(const SkDraw&, const void* text, size_t len,
                             const SkScalar pos[], SkScalar constY,
                             int scalarsPerPos, const SkPaint&) SK_OVERRIDE;
    virtual void drawTextOnPath(const SkDraw&, const void* text, size_t len,
                                const SkPath& path, const SkMatrix* matrix,
                                const SkPaint&) SK_OVERRIDE;
    virtual void drawVertices(const SkDraw&, SkCanvas::VertexMode, int vertexCount,
                              const SkPoint verts[], const SkPoint texs[],
                              const SkColor colors[], SkXfermode* xmode,
                              const uint16_t indices[], int indexCount,
                              const SkPaint&) SK_OVERRIDE;
    virtual void drawDevice(const SkDraw&, SkDevice*, int x, int y,
                            const SkPaint&) SK_OVERRIDE;
    virtual bool filterTextFlags(const SkPaint&, TextFlags*) SK_OVERRIDE;

    virtual void flush(); 

    /**
     * Make's this device's rendertarget current in the underlying 3D API.
     * Also implicitly flushes.
     */
    virtual void makeRenderTargetCurrent();

protected:
    typedef GrContext::TextureCacheEntry TexCache;
    enum TexType {
        kBitmap_TexType,
        kDeviceRenderTarget_TexType,
        kSaveLayerDeviceRenderTarget_TexType
    };
    TexCache lockCachedTexture(const SkBitmap& bitmap,
                               const GrSamplerState& sampler,
                               TexType type = kBitmap_TexType);
    bool isBitmapInTextureCache(const SkBitmap& bitmap,
                                const GrSamplerState& sampler) const;
    void unlockCachedTexture(TexCache);

    class SkAutoCachedTexture {
    public:
        SkAutoCachedTexture();
        SkAutoCachedTexture(SkGpuDevice* device,
                            const SkBitmap& bitmap,
                            const GrSamplerState& sampler,
                            GrTexture** texture);
        ~SkAutoCachedTexture();

        GrTexture* set(SkGpuDevice*, const SkBitmap&, const GrSamplerState&);

    private:
        SkGpuDevice*    fDevice;
        TexCache        fTex;
    };
    friend class SkAutoTexCache;
    
    // overrides from SkDevice
    virtual bool onReadPixels(const SkBitmap& bitmap,
                              int x, int y,
                              SkCanvas::Config8888 config8888) SK_OVERRIDE;


private:
    GrContext*      fContext;

    GrSkDrawProcs*  fDrawProcs;

    // state for our offscreen render-target
    TexCache            fCache;
    GrTexture*          fTexture;
    GrRenderTarget*     fRenderTarget;
    bool                fNeedClear;
    bool                fNeedPrepareRenderTarget;

    // called from rt and tex cons
    void initFromRenderTarget(GrContext*, GrRenderTarget*);

    // doesn't set the texture/sampler/matrix state
    // caller needs to null out GrPaint's texture if
    // non-textured drawing is desired.
    // Set constantColor to true if a constant color
    // will be used.  This is an optimization, and can 
    // always be set to false. constantColor should 
    // never be true if justAlpha is true.
    bool skPaint2GrPaintNoShader(const SkPaint& skPaint,
                                 bool justAlpha,
                                 GrPaint* grPaint,
                                 bool constantColor);

    // uses the SkShader to setup paint, act used to
    // hold lock on cached texture and free it when
    // destroyed.
    // If there is no shader, constantColor will
    // be passed to skPaint2GrPaintNoShader.  Otherwise
    // it is ignored.
    bool skPaint2GrPaintShader(const SkPaint& skPaint,
                               SkAutoCachedTexture* act,
                               const SkMatrix& ctm,
                               GrPaint* grPaint,
                               bool constantColor);

    // override from SkDevice
    virtual SkDevice* onCreateCompatibleDevice(SkBitmap::Config config, 
                                               int width, int height, 
                                               bool isOpaque,
                                               Usage usage);

    SkDrawProcs* initDrawForText(GrTextContext*);
    bool bindDeviceAsTexture(GrPaint* paint);

    void prepareRenderTarget(const SkDraw&);
    bool shouldTileBitmap(const SkBitmap& bitmap,
                          const GrSamplerState& sampler,
                          const SkIRect* srcRectPtr,
                          int* tileSize) const;
    void internalDrawBitmap(const SkDraw&, const SkBitmap&,
                            const SkIRect&, const SkMatrix&, GrPaint* grPaint);

    typedef SkDevice INHERITED;
};

#endif

