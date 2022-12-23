//
// Created by maxbr on 21.11.2022.
//

#pragma once

#include "Defs.h"
#include "State.h"
#include <vector>
#include "Engine/Drawable.h"
#include "Objects/ColorfullTriangle.h"
#include "Objects/Terrain.h"

class Scene
{
public:
    Scene(std::shared_ptr<State> state);
    ~Scene();

    void update(float deltaTime);
    void renderDrawables();

    void addObject(std::shared_ptr<Drawable> object);

private:

    std::vector<std::shared_ptr<Drawable>> m_drawables;
    std::unique_ptr<float[]> m_backgroundColor;   
    std::shared_ptr<State> m_state;

    // Objects
    std::shared_ptr<ColorfullTriangle> m_triangle;
    std::shared_ptr<Drawable> m_terrain;

};