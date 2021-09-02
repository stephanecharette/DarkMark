// DarkMark (C) 2019-2021 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"


// This is part of a sponsored change to provide a simplified "darknet" window
// with less options than normal.  This is also used to trigger the Japanese
// translations.  Usually, this flag is set to "false" which bypasses most of
// the code in this file.
#if DARKNET_GEN_SIMPLIFIED
constexpr bool simplified_interface = true;
#else
constexpr bool simplified_interface = false;
#endif
constexpr bool normal_interface = not simplified_interface;


String dm::getText(String key)
{
	if (normal_interface)
	{
		if (key == "TITLE")
		{
			return "DarkMark v" DARKMARK_VERSION " - Darknet Output";
		}
		if (key == "TITLE2")
		{
			return "Importing .txt files...";
		}
		if (key == "TITLE3")
		{
			return "Saving Darknet Files...";
		}

		return key;
	}

	const std::map<String, String> m =
	{
//		{"TITLE"									, String::fromUTF8("学習設定画面")						},
		{"TITLE"									, "DNNM v" DARKMARK_VERSION								},
		{"configuration"							, String::fromUTF8("設定")								},
		{"network width"							, String::fromUTF8("画像横幅（Px）")						},
		{"network height"							, String::fromUTF8("画像縦幅（Px）")						},
		{"batch size"								, String::fromUTF8("バッチサイズ")						},
		{"subdivisions"								, String::fromUTF8("分割数")								},
		{"max_batches"								, String::fromUTF8("学習回数")							},
		{"learning_rate"							, String::fromUTF8("学習率")								},
		{"images"									, String::fromUTF8("画像処理")							},
		{"do not resize images"						, String::fromUTF8("拡大縮小しない")						},
		{"resize images"							, String::fromUTF8("縦横幅に合わせて縮小")					},
		{"tile images"								, String::fromUTF8("縦横幅に合わせてタイリング")				},
		{"crop & zoom images"						, String::fromUTF8("画像のトリミングとズーム")				},
		{"limit negative samples"					, String::fromUTF8("マークなしサンプル数を抑制")				},
		{"train with all images"					, String::fromUTF8("全画像を学習に用いる")					},
		{"training images %"						, String::fromUTF8("学習用に使用する画像の数量（%）")			},
		{"yolo"										, String::fromUTF8("archY")								},
		{"recalculate yolo anchors"					, String::fromUTF8("アンカーの再設定")						},
		{"data augmentation [colour]"				, String::fromUTF8("データオーグメンテーション [色]")		},
		{"saturation"								, String::fromUTF8("彩度")								},
		{"exposure"									, String::fromUTF8("明度")								},
		{"hue"										, String::fromUTF8("色相")								},
		{"data augmentation [misc]"					, String::fromUTF8("データオーグメンテーション [その他]")		},
		{"enable flip"								, String::fromUTF8("左右反転")							},
		{"enable mosaic"							, String::fromUTF8("モザイク")							},
		{"Cancel"									, String::fromUTF8("キャンセル")							},
		{"OK"										, String::fromUTF8("決定")								},

		{"Looking for images and annotations..."	, String::fromUTF8("アノテーションをインポート中")			},

		{"Finding all images and annotations..."	, String::fromUTF8("全ての画像とアノテーションを検索中...")	},
		{"Listing skipped images..."				, String::fromUTF8("スキップした画像リストを生成中...")		},
		{"Resizing images to"						, String::fromUTF8("へ画像を縮小中")						},
		{"Tiling images to"							, String::fromUTF8("へ画像をタイリング中")					},
		{"Random image crop and zoom..."			, String::fromUTF8("画像の切り出しと拡大中...")				},
		{"Recalculating anchors..."					, String::fromUTF8("アンカーを設定中...")					},
		{"Limit negative samples..."				, String::fromUTF8("マークなしサンプル数を削減中...")		},
		{"Writing training and validation files..."	, String::fromUTF8("学習用ファイルを書き出し中...")			},
		{"Creating training and validation files...", String::fromUTF8("学習用ファイルを生成中...")				},
		{"Creating configuration files and shell scripts...", String::fromUTF8("設定ファイルを生成中...")		}
	};

	const std::map<String, String> special_cases =
	{
		{"TITLE2"										, "TITLE"									},
		{"TITLE3"										, "TITLE"									},
		{"resize images to match the network dimensions", "resize images"							},
		{"tile images to match the network dimensions"	, "tile images"								},
		{"random crop and zoom images"					, "crop & zoom images"						},
		{"flip (left-right)"							, "enable flip"								},
		{"mosaic"										, "enable mosaic"							},
		{"Building image list..."						, "Looking for images and annotations..."	},
	};
	if (special_cases.count(key))
	{
		key = special_cases.at(key);
	}

	if (m.count(key))
	{
		return m.at(key);
	}

	return key;
}
