local spr = app.activeSprite
if not spr then return print('No active sprite') end

local local_path,title = spr.filename:match("^(.+[/\\])(.-).([^.]*)$")

local home = os.getenv("HOME")
local export_path = home .. "/prog/personal/ooga/assets"
local_path = export_path

function layer_export()
  local fn = local_path .. "/" .. app.activeLayer.name
  app.command.ExportSpriteSheet{
      ui=false,
      type=SpriteSheetType.HORIZONTAL,
      textureFilename=fn .. '.png',
      dataFormat=SpriteSheetDataFormat.JSON_ARRAY,
      layer=app.activeLayer.name,
      trim=true,
  }
end

layer_export()
