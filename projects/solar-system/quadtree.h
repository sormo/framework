#pragma once
#include "bodies_tree.h"

struct quadtree
{
    struct quadnode
    {
        quadnode(frame::rectangle&& r) : rect(std::move(r)) {}

        frame::rectangle rect;

        std::vector<body_data*> trajectories;

        std::array<std::unique_ptr<quadnode>, 4> childs;
    };

    std::unique_ptr<quadnode> root;

    void construct(frame::rectangle root_rect, float min_size, bodies_tree& data)
    {
        root = std::make_unique<quadnode>(std::move(root_rect));

        for (auto& body : data.bodies)
        {
            for (auto& p : body.trajectory.get_points())
            {
                if (root->rect.contains(p))
                {
                    root->trajectories.push_back(&body);
                    break;
                }
            }
        }

        construct_recursive(root.get(), min_size);
    }

    void draw()
    {
        draw_recursive(root.get());
    }

private:

    void draw_recursive(quadnode* node)
    {
        //if (!node->childs[0] || !node->childs[1] || !node->childs[2] || !node->childs[3])
        frame::draw_rectangle_ex(node->rect.center(), 0.0f, node->rect.size().x, node->rect.size().y, frame::col4::BLANK, commons::scale_independent(1.0f), frame::col4::RED);
        if (node->childs[0])
            draw_recursive(node->childs[0].get());
        if (node->childs[1])
            draw_recursive(node->childs[1].get());
        if (node->childs[2])
            draw_recursive(node->childs[2].get());
        if (node->childs[3])
            draw_recursive(node->childs[3].get());
    }

    std::unique_ptr<quadnode> construct_child(frame::rectangle rect, float min_size, std::vector<body_data*>& parent_trajectories)
    {
        std::unique_ptr<quadnode> result = std::make_unique<quadnode>(std::move(rect));

        for (auto& body : parent_trajectories)
        {
            for (auto& p : body->trajectory.get_points())
            {
                if (result->rect.contains(p))
                {
                    result->trajectories.push_back(body);
                    break;
                }
            }
        }

        if (result->trajectories.empty())
            return nullptr;

        construct_recursive(result.get(), min_size);

        return result;
    }

    void construct_recursive(quadnode* node, float min_size)
    {
        auto hsize = node->rect.size() / 2.0f;

        if (std::min(hsize.x, hsize.y) <= min_size / 2.0f)
            return;

        auto right = frame::vec2{ node->rect.min.x + hsize.x, node->rect.min.y };
        auto up = frame::vec2{ node->rect.min.x, node->rect.min.y + hsize.y };

        node->childs[0] = construct_child(frame::rectangle::from_min_max(node->rect.min, node->rect.min + hsize), min_size, node->trajectories);
        node->childs[1] = construct_child(frame::rectangle::from_min_max(right, right + hsize), min_size, node->trajectories);
        node->childs[2] = construct_child(frame::rectangle::from_min_max(up, up + hsize), min_size, node->trajectories);
        node->childs[3] = construct_child(frame::rectangle::from_min_max(node->rect.min + hsize, node->rect.max), min_size, node->trajectories);
    }
};
