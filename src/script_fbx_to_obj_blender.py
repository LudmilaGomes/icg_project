import bpy
import os

path_fbx_files = "/home/ludmila/projeto_icg/LowPolyWildWest/"


def fbx_to_obj_blender(path_in: str, path_out: str):
  if not os.path.exists(path_out):
    os.makedirs(path_out)

  list_fbxs = [f for f in os.listdir(path_in) if f.lower().endswith(".fbx")]

  for current_fbx in list_fbxs:
    # deleta objeto anterior
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete()

    # limpa dados 'orfãos'
    bpy.ops.outliner.orphans_purge(
      do_local_ids=True, do_linked_ids=True, do_recursive=True
    )

    # importa .fbx
    path = os.path.join(path_in, current_fbx)
    bpy.ops.import_scene.fbx(filepath=path)
    bpy.context.view_layer.update()

    output_filename = current_fbx.replace(".fbx", ".obj")
    output_path = os.path.join(path_out, output_filename)

    try:
      bpy.ops.object.select_all(action="SELECT")

      bpy.ops.wm.obj_export(
        filepath=output_path,
        export_selected_objects=False,
        apply_modifiers=True,
        export_normals=True,
        export_uv=True,
        export_materials=True,
        export_triangulated_mesh=True,
        path_mode="RELATIVE",
      )
      print(f"Sucesso: {current_fbx}")
    except Exception as e:
      print(f"Falha no arquivo {current_fbx}: {e}")


for root, _, files in os.walk(path_fbx_files):
  if any(f.lower().endswith(".fbx") for f in files):
    
    if not root.endswith("_obj_mtl"):
      root_out = root.rstrip("/") + "_obj_mtl"
      fbx_to_obj_blender(root, root_out)

print("Processo finalizado.")
