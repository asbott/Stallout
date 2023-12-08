#pragma once //

#include "Stallout/maths.h"

NS_BEGIN(stallout);
NS_BEGIN(graphics);

struct AABB {
    AABB() {
        a1 = a2 = b1= b2 = 0;
    }
    AABB(mz::fvec2 a1, mz::fvec2 a2 = 0, mz::fvec2 b1 = 0, mz::fvec2 b2 = 0)
        : a1(a1), b1(b1), a2(a2), b2(b2) {
        
    }
    AABB(f32 a1x, f32 a1y = 0, f32 a2x = 0, f32 a2y = 0, 
         f32 b1x = 0, f32 b1y = 0, f32 b2x = 0, f32 b2y = 0)
        : a1x(a1x), a1y(a1y), a2x(a2x), a2y(a2y),
          b1x(b1x), b1y(b1y), b2x(b2x), b2y(b2y) {
        
    }
    union {
        struct {
            mz::fvec2 a1, a2, b1, b2;    
        };
        struct {
            f32 a1x, a1y, a2x, a2y,
                b1x, b1y, b2x, b2y;
        };
        mz::fvec2 ptr[4];
    };
};

struct Camera_Transform2D {
    // View
    mz::fvec2 position = 0;
    f32 rotation = 0;
    mz::fvec2 zoom = 0;

    // Projection
    
    union {
        mz::fvec2 size = { 1280, 720 };
        struct { f32 width; f32 height; };
    };

    mz::fmat4 get_ortho() const {
        return mz::projection::ortho<f32>(0.f, (width), 0.f, (height), -10000.f, 10000.f);
    }
    mz::fmat4 get_view() const {
        mz::fmat4 view = mz::transformation::translation<f32>({ (position), 0 });
        view.rotate(rotation, { 0, 0, -1 });
        view.scale(-zoom);
        view.translate((mz::fvec3{ -width / 2.f, -height / 2.f, 0 }));
        view.invert(); 

        return view;
    }

    mz::fmat4 get_transform() const {
        return get_ortho() * get_view();
    }

    mz::fvec2 get_scale() const {
        return mz::fvec2(1) - zoom;
    }
};

NS_END(graphics);
NS_END(stallout);

