import numpy as np
from common import compose_transform_matrix as transform_compose, decompose_transform_matrix as transform_decompose
from slayer_bindings.glm import transform_compose, transform_decompose


def process_animation(channels, bone_data: dict, inv_transform: np.ndarray):
    # We wan't to treverse the bone hierarchy for each animation channel so that each animation frame is in mesh space
    # This is done by multiplying the bone's local transform with the parent's transform

    channels = sorted(
        channels, key=lambda channel: bone_data[channel["node_name"]]["parent_id"])

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
        pos = np.array(pos_key[1:])
        rot = np.array(rot_key[1:])
        scale = np.array(scale_key[1:])

        parent = parent_transforms[t] if parent_transforms is not None else np.identity(
            4)

        local_transform = transform_compose(pos, rot, scale)
        model_transform = parent @ local_transform
        matrices[t] = model_transform
        new_bone_mat = inv_transform @ model_transform

        new_pos, new_rot, new_scale = transform_decompose(new_bone_mat)

        new_position_keys.append([t, *new_pos])
        new_rotation_keys.append([t, *new_rot])
        new_scale_keys.append([t, *new_scale])

    return {"node_name": node_name, "position_keys": new_position_keys,
            "rotation_keys": new_rotation_keys, "scale_keys": new_scale_keys}, matrices
