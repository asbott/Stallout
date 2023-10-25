#pragma once

#include <mz_vector.hpp>
#include <mz_matrix.hpp>

NS_BEGIN(engine);
NS_BEGIN(graphics);

struct Camera_Transform2D {
    // View
    mz::fvec2 position = 0;
    f32 rotation = 0;
    mz::fvec2 zoom = 0;

    // Projection
    
    f32 width = 1280, height = 720;

    mz::fmat4 get_ortho() const {
        return mz::projection::ortho<f32>(0.f, width, 0.f, height, -100.f, 100.f);
    }
    mz::fmat4 get_view() const {
        mz::fmat4 view = mz::transformation::translation<f32>({ position, 0 });
        view.rotate(rotation, { 0, 0, -1 });
        view.scale(-zoom);
        view.translate({ -width / 2.f, -height / 2.f, 0 });
        view.invert(); 

        return view;
    }

    mz::fmat4 get_transform() const {
        return get_ortho() * get_view();
    }
};

NS_END(graphics);
NS_END(engine);

