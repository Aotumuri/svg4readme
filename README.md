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

## ビルド方法
config.cを編集したら以下のコマンドを実行する必要があります。
```bash
gcc -O2 waves_svg.c config.c -lm -o waves_svg
gcc -O2 fire_svg.c config.c -lm -o fire_svg
```

## 実行方法
以下のコマンドでsvgが生成されます。
```bash
./waves_svg > waves.svg
./fire_svg > fire.svg 
```

## 出力例
生成例：
wave.svg (夜の海)
<img src="https://raw.githubusercontent.com/Aotumuri/svg4readme/refs/heads/main/sample/waves.svg" />

fire.svg (噴火した火山)
<img src="https://raw.githubusercontent.com/Aotumuri/svg4readme/refs/heads/main/sample/fire.svg" />
