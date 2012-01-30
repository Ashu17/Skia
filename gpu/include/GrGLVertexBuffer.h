/*
    Copyright 2011 Google Inc.
    Copyright (C) 2011 Research In Motion Limited. All rights reserved.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#ifndef GrGLVertexBuffer_DEFINED
#define GrGLVertexBuffer_DEFINED

#include "GrVertexBuffer.h"
#include "GrGLInterface.h"

class GrGpuGL;

class GrGLVertexBuffer : public GrVertexBuffer {

public:
    virtual ~GrGLVertexBuffer() { this->release(); }
    // overrides of GrVertexBuffer
    virtual void* lock();
    virtual void* lockPtr() const;
    virtual void unlock();
    virtual bool isLocked() const;
    virtual bool updateData(const void* src, size_t srcSizeInBytes);
    virtual bool updateSubData(const void* src,
                               size_t srcSizeInBytes,
                               size_t offset);
protected:
    GrGLVertexBuffer(GrGpuGL* gpu,
                     void* ptr,
                     size_t sizeInBytes,
                     bool dynamic);

    // overrides of GrResource
    virtual void onAbandon();
    virtual void onRelease();

private:
    void* fPtr;
    bool fLocked;

    friend class GrGpuGL;

    typedef GrVertexBuffer INHERITED;
};

#endif
