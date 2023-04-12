from slayer_bindings.assimp import load_bone_data

print("Loading bone data...")
vertices, indices, bones = load_bone_data("C:\dev\repos\Slayer\Testbed\assets\Mannequin\SK_Mannequin.fbx")
print(len(vertices))
print(len(indices))
print(list(bones.keys()))