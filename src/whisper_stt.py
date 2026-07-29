#!/usr/bin/env python3
"""
Whisper 语音识别脚本 — 使用 faster-whisper 离线识别中文语音

用法：
    python3 whisper_stt.py <model_size> <wav_file_path>

参数：
    model_size   : tiny / base / small / medium（推荐 small 平衡速度与精度）
    wav_file_path: 16kHz 16-bit 单声道 WAV 文件路径

输出：
    识别到的中文文本（stdout）
    错误信息（stderr）

返回码：
    0 = 成功
    1 = 参数错误
    2 = 识别失败

依赖：
    pip3 install faster-whisper

模型缓存位置：
    ~/.cache/huggingface/hub/models--Systran--faster-whisper-<size>/
"""

import sys
import os
import time


def main():
    if len(sys.argv) != 3:
        print("用法: python3 whisper_stt.py <model_size> <wav_file>", file=sys.stderr)
        sys.exit(1)

    model_size = sys.argv[1]
    wav_file = sys.argv[2]

    # 展开 ~ 路径
    model_size = os.path.expanduser(model_size)

    # 如果是本地目录路径 → 直接使用；否则验证是否为有效模型名
    valid_sizes = ("tiny", "base", "small", "medium", "large")
    if not os.path.isdir(model_size) and model_size not in valid_sizes:
        print(f"无效的模型: {model_size}，可选: {', '.join(valid_sizes)} 或本地模型目录",
              file=sys.stderr)
        sys.exit(1)

    # 验证 WAV 文件
    if not os.path.isfile(wav_file):
        print(f"WAV 文件不存在: {wav_file}", file=sys.stderr)
        sys.exit(1)

    file_size = os.path.getsize(wav_file)
    if file_size == 0:
        print("WAV 文件为空", file=sys.stderr)
        sys.exit(1)

    try:
        from faster_whisper import WhisperModel
    except ImportError:
        print("faster-whisper 未安装。请运行: pip3 install faster-whisper",
              file=sys.stderr)
        sys.exit(2)

    # ===== 加载模型 =====
    # 模型大小参考：
    #   tiny   ~39MB — 最快，精度一般
    #   base   ~74MB — 平衡
    #   small  ~466MB — 推荐，中文效果好
    #   medium ~1.5GB — 更准确但更慢
    try:
        model = WhisperModel(
            model_size,
            device="cpu",           # 使用 CPU（VM 没有 GPU）
            compute_type="int8",    # int8 量化节省内存，CPU 上速度也不错
            cpu_threads=2,          # VM 内存小，限制线程数
            num_workers=1
        )
    except Exception as e:
        print(f"模型加载失败: {e}", file=sys.stderr)
        sys.exit(2)

    # ===== 执行识别 =====
    # 中文语音识别参数：
    #   language="zh"   : 指定中文，减少误识别
    #   task="transcribe": 转写（同语言），非翻译
    #   beam_size=3     : beam search 宽度（大→更准但更慢）
    try:
        segments, info = model.transcribe(
            wav_file,
            language="zh",
            task="transcribe",
            beam_size=3,
            vad_filter=True,       # 过滤静音段
            vad_parameters=dict(
                min_silence_duration_ms=500
            )
        )

        # 拼接所有分段
        text_parts = []
        for segment in segments:
            text_parts.append(segment.text.strip())

        result = " ".join(text_parts).strip()

        if not result:
            print("（未识别到语音内容）", file=sys.stderr)
            sys.exit(2)

        # 输出识别结果到 stdout
        print(result)
        sys.exit(0)

    except Exception as e:
        print(f"语音识别失败: {e}", file=sys.stderr)
        sys.exit(2)


if __name__ == "__main__":
    main()
