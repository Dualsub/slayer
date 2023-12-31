import numpy as np
import json
from common import compose_transform_matrix as transform_compose, decompose_transform_matrix as transform_decompose
# from slayer_bindings.glm import transform_compose, transform_decompose


def process_animation(channels, bone_data: dict, inv_transform: np.ndarray):
    return channels
    new_channels = []
    # Change all rotations to be in order yzwx
    for channel in channels:
        new_channel = {}
        new_channel["node_name"] = channel["node_name"]
        new_channel["position_keys"] = channel["position_keys"]
        new_channel["scale_keys"] = channel["scale_keys"]
        new_channel["rotation_keys"] = []
        for rot_key in channel["rotation_keys"]:
            new_rot_key = (rot_key[0], rot_key[2],
                           rot_key[3], rot_key[4], rot_key[1])
            new_channel["rotation_keys"].append(new_rot_key)
        new_channels.append(new_channel)

    return new_channels


def old_process_animation(channels, bone_data: dict, inv_transform: np.ndarray):
    channels = sorted(
        channels, key=lambda channel: bone_data[channel["node_name"]]["parent_id"] if channel["node_name"] in bone_data else -1)

    id_to_name = {bone["id"]: bone_name
                  for bone_name, bone in bone_data.items()}

    # Assert topological sort
    visited = set()
    for channel in channels:
        node_name = channel["node_name"]
        bone = bone_data[node_name]
        parent_id = bone["parent_id"]
        visited.add(node_name)
        if parent_id == -1:
            continue
        parent_name = id_to_name[parent_id]
        assert parent_name in visited

    new_channels = []
    # Bones that have been multiplied with their parent's transform
    nodes = dict()

    for channel in channels:
        node_name = channel["node_name"]
        if node_name not in bone_data:
            new_channels.append(channel)
            continue
        bone = bone_data[node_name]
        parent_id = bone["parent_id"]
        if parent_id == -1:
            new_channel, matrices = compute_node(
                channel, node_name, bone, inv_transform)
            new_channels.append(new_channel)
            nodes[node_name] = matrices
        else:
            parent_name = id_to_name[parent_id]
            assert parent_name in nodes.keys()

            parent_name = id_to_name[parent_id]
            parent_transforms = nodes[parent_name]
            new_channel, matrices = compute_node(
                channel, node_name, bone, inv_transform, parent_transforms)
            new_channels.append(new_channel)
            nodes[node_name] = matrices

    return new_channels


def compute_node(channel, node_name, bone, inv_transform, parent_transforms=None):
    new_position_keys = []
    new_rotation_keys = []
    new_scale_keys = []
    matrices = {}
    for pos_key, rot_key, scale_key in zip(channel["position_keys"], channel["rotation_keys"], channel["scale_keys"]):
        assert pos_key[0] == rot_key[0] == scale_key[0]
        t = pos_key[0]
        t_key = str(round(t, 6))
        pos = np.array(pos_key[1:])
        # rot yzwx
        rot = np.array([rot_key[2], rot_key[3], rot_key[4], rot_key[1]])
        scale = np.array(scale_key[1:])

        # parent = parent_transforms[t_key] if parent_transforms is not None else np.identity(
        #     4)

        local_transform = transform_compose(pos, rot, scale)
        # model_transform = inv_transform @ parent @ local_transform
        # matrices[t_key] = model_transform
        new_bone_mat = local_transform

        new_pos, new_rot, new_scale = transform_decompose(new_bone_mat)

        new_position_keys.append([t, *new_pos])
        new_rotation_keys.append([t, *new_rot])
        new_scale_keys.append([t, *new_scale])

    return {"node_name": node_name, "position_keys": new_position_keys,
            "rotation_keys": new_rotation_keys, "scale_keys": new_scale_keys}, matrices
