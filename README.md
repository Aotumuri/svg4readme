# svg4readme

## 概要
`svg4readme` は、夜空・月・波・ブランド名などを合成した SVG バナーを生成するプログラムです。
README のヘッダー画像やプロフィール装飾に利用できます。

## 特徴
共通
- 名前や使用言語などを出力

Waves
- 星の瞬きと流れ星アニメーション
- グラデーションの空と発光する月
- 複数レイヤーの波のゆらぎ

Fire
- 煙を出している火山
- 降ってくる火の粉のアニメーション
- 複数レイヤーの波の揺らぎ（溶岩）

Forest
- 月明かりに照らされた針葉樹の森
- たなびく霧と、ゆっくり明滅する蛍

## プロフィールの入力

まず `profile.conf.example` を `profile.conf` にコピーして、表示したい内容を書き換えます。`profile.conf` は実行時に読むので、内容を変えても再ビルドは不要です。値に空白を含めてもそのまま使えます。

```bash
cp profile.conf.example profile.conf
# profile.conf を編集
```

各説明欄は最大3行、見出しは最大2行まで自動で折り返します。収まりきらない場合は末尾を `…` にして、バナーの外にはみ出さないようにします。改行を入れれば任意の位置で改行できます。

一時的に内容を差し替えたい場合は、従来どおり環境変数も使えます（`profile.conf` より優先）。

```bash
BRAND_NAME='Your Name' FIRST_DESC1='Rust / Go / TypeScript' make waves
```

## ビルド方法

```bash
make
```

個別にビルドする場合は以下です。

```bash
gcc -O2 waves_svg.c config.c -lm -o waves_svg
gcc -O2 fire_svg.c config.c -lm -o fire_svg
gcc -O2 forest_svg.c config.c -o forest_svg
```

## 実行方法
以下のコマンドで SVG が生成されます。
```bash
make waves
make fire
make forest
```

## 出力例
生成例：
wave.svg (夜の海)
<img src="https://raw.githubusercontent.com/Aotumuri/svg4readme/refs/heads/main/sample/waves.svg" />

fire.svg (噴火した火山)
<img src="https://raw.githubusercontent.com/Aotumuri/svg4readme/refs/heads/main/sample/fire.svg" />

forest.svg (夜の森)
<img src="https://raw.githubusercontent.com/Aotumuri/svg4readme/refs/heads/main/sample/forest.svg" />
