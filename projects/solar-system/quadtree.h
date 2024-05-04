#pragma once
#include <set>
#include "bodies_tree.h"
#include <json.hpp>

struct quadtree
{
    struct quadnode
    {
        quadnode() {}
        quadnode(frame::rectangle&& r) : rect(std::move(r)) {}

        frame::rectangle rect;

        std::vector<body_data*> bodies;

        std::array<std::unique_ptr<quadnode>, 4> childs;

        bool has_any_child() const
        {
            return (bool)childs[0] || (bool)childs[1] || (bool)childs[2] || (bool)childs[3];
        }
    };

    std::unique_ptr<quadnode> root;

    void construct(frame::rectangle root_rect, float min_size, bodies_tree& data)
    {
        root = std::make_unique<quadnode>(std::move(root_rect));

        std::vector<size_t> indices; // optimization, provide indices from which should be bodies iterated to childs
        for (auto& body : data.bodies)
        {
            // we need to pick only bodies which has parent solar-system barycenter, this barycenter does not move
            // so also it's child bodies are stationary
            if (body.parent && body.parent->name != "Solar-System Barycenter")
                continue;

            for (size_t j = 0; j < body.trajectory.get_points().size(); j++)
            {
                if (root->rect.contains(body.trajectory.get_points()[j]))
                {
                    indices.push_back(j);
                    root->bodies.push_back(&body);
                    break;
                }
            }
        }

        construct_recursive(root.get(), min_size, indices);
    }

    std::set<body_data*> query(const frame::rectangle& rect)
    {
        std::set<body_data*> result;

        query_recursive(root.get(), rect, result);

        return result;
    }

    void draw_debug()
    {
        draw_debug_recursive(root.get());
    }

    nlohmann::json serialize()
    {
        return serialize_recursive(*root);
    }

    bool deserialize(nlohmann::json& json, std::vector<body_data>& bodies)
    {
        std::map<std::string, body_data*> bodies_map;
        for (auto& body : bodies)
            bodies_map.emplace(body.name, &body);

        root = std::make_unique<quadnode>();

        return deserialize_recursive(json, *root, bodies_map);
    }

private:
    void query_recursive(quadnode* node, const frame::rectangle& rect, std::set<body_data*>& result)
    {
        if (!node)
            return;

        if (rect.contains(node->rect))
        {
            for (auto body : node->bodies)
                result.insert(body);
        }
        else if (rect.has_overlap(node->rect))
        {
            if (node->has_any_child())
            {
                query_recursive(node->childs[0].get(), rect, result);
                query_recursive(node->childs[1].get(), rect, result);
                query_recursive(node->childs[2].get(), rect, result);
                query_recursive(node->childs[3].get(), rect, result);
            }
            else
            {
                for (auto body : node->bodies)
                    result.insert(body);
            }
        }
    }

    void draw_debug_recursive(quadnode* node)
    {
        if (!node)
            return;

        //if (!node->childs[0] || !node->childs[1] || !node->childs[2] || !node->childs[3])
        frame::draw_rectangle_ex(node->rect.center(), 0.0f, node->rect.size().x, node->rect.size().y, frame::col4::BLANK, commons::scale_independent(1.0f), frame::col4::RED);
        if (node->has_any_child())
        {
            draw_debug_recursive(node->childs[0].get());
            draw_debug_recursive(node->childs[1].get());
            draw_debug_recursive(node->childs[2].get());
            draw_debug_recursive(node->childs[3].get());
        }
    }

    nlohmann::json serialize_recursive(quadnode& node)
    {
        nlohmann::json result;

        result["minx"] = node.rect.min.x;
        result["miny"] = node.rect.min.y;
        result["maxx"] = node.rect.max.x;
        result["maxy"] = node.rect.max.y;
        
        auto bodies = nlohmann::json::array();
        for (auto& body : node.bodies)
            bodies.push_back(body->name);

        result["bodies"] = bodies;

        if (node.childs[0])
            result["child0"] = serialize_recursive(*node.childs[0]);
        if (node.childs[1])
            result["child1"] = serialize_recursive(*node.childs[1]);
        if (node.childs[2])
            result["child2"] = serialize_recursive(*node.childs[2]);
        if (node.childs[3])
            result["child3"] = serialize_recursive(*node.childs[3]);

        return result;
    }

    bool deserialize_recursive(nlohmann::json& json, quadnode& node, std::map<std::string, body_data*>& bodies)
    {
        if (!json.contains("minx") || !json.contains("miny") || !json.contains("maxx") || !json.contains("maxy") || !json.contains("bodies"))
            return false;

        auto min = frame::vec2(json["minx"], json["miny"]);
        auto max = frame::vec2(json["maxx"], json["maxy"]);
        node.rect = frame::rectangle::from_min_max(min, max);

        for (const std::string& name : json["bodies"])
        {
            if (bodies.count(name))
                node.bodies.push_back(bodies[name]);
        }

        auto deserialize_child = [this](const char* name, nlohmann::json& json, std::unique_ptr<quadnode>& node, std::map<std::string, body_data*>& bodies)
        {
            if (!json.contains(name))
                return true;
            node = std::make_unique<quadnode>();
            return deserialize_recursive(json[name], *node, bodies);
        };

        if (!deserialize_child("child0", json, node.childs[0], bodies))
            return false;
        if (!deserialize_child("child1", json, node.childs[1], bodies))
            return false;
        if (!deserialize_child("child2", json, node.childs[2], bodies))
            return false;
        if (!deserialize_child("child3", json, node.childs[3], bodies))
            return false;

        return true;
    }

    std::unique_ptr<quadnode> construct_child(frame::rectangle rect, float min_size, std::vector<body_data*>& parent_bodies, const std::vector<size_t>& parent_indices)
    {
        std::unique_ptr<quadnode> result = std::make_unique<quadnode>(std::move(rect));

        std::vector<size_t> indices; // optimization, provide indices from which should be bodies iterated to childs
        for (size_t i = 0; i < parent_bodies.size(); i++)
        {
            auto& body = parent_bodies[i];

            for (size_t j = parent_indices[i]; j < body->trajectory.get_points().size(); j++)
            {
                if (result->rect.contains(body->trajectory.get_points()[j]))
                {
                    indices.push_back(j);
                    result->bodies.push_back(body);
                    break;
                }
            }
        }

        if (result->bodies.empty())
            return nullptr;

        construct_recursive(result.get(), min_size, indices);

        return result;
    }

    void construct_recursive(quadnode* node, float min_size, const std::vector<size_t>& parent_indices)
    {
        auto hsize = node->rect.size() / 2.0f;

        if (std::min(hsize.x, hsize.y) <= min_size / 2.0f)
            return;

        auto right = frame::vec2{ node->rect.min.x + hsize.x, node->rect.min.y };
        auto up = frame::vec2{ node->rect.min.x, node->rect.min.y + hsize.y };

        // either all childs or none, is this good ?
        node->childs[0] = construct_child(frame::rectangle::from_min_max(node->rect.min, node->rect.min + hsize), min_size, node->bodies, parent_indices);
        node->childs[1] = construct_child(frame::rectangle::from_min_max(right, right + hsize), min_size, node->bodies, parent_indices);
        node->childs[2] = construct_child(frame::rectangle::from_min_max(up, up + hsize), min_size, node->bodies, parent_indices);
        node->childs[3] = construct_child(frame::rectangle::from_min_max(node->rect.min + hsize, node->rect.max), min_size, node->bodies, parent_indices);
    }
};
