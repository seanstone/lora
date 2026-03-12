/// LoRa link simulation: burst generator → AWGN channel → receiver.
///
/// Usage: link_sim [sf] [snr_db] [n_packets] [interval_ms]
/// Defaults: sf=7  snr=10  n=20  interval=250

use lora::tx::{
    whitening::whiten, header::add_header, crc::add_crc,
    hamming_enc::hamming_enc, interleaver::interleave,
    gray_demap::gray_demap, modulate::modulate,
};
use lora::rx::{
    frame_sync::frame_sync, fft_demod::fft_demod,
    gray_mapping::gray_map, deinterleaver::deinterleave,
    hamming_dec::hamming_dec, header_decoder::decode_header,
    crc_verif::verify_crc, dewhitening::dewhiten,
};
use rand::{Rng, SeedableRng};
use rustfft::num_complex::Complex;
use std::time::{Duration, Instant};

// ─── AWGN channel ────────────────────────────────────────────────────────────

/// Box-Muller: unit-normal sample using two uniform draws.
fn randn(rng: &mut impl Rng) -> f32 {
    // `gen` is a reserved keyword in edition 2024; use r#gen to escape it.
    let u1 = (rng.r#gen::<f32>() + 1e-7_f32).min(1.0);
    let u2 = rng.r#gen::<f32>();
    (-2.0 * u1.ln()).sqrt() * (std::f32::consts::TAU * u2).cos()
}

/// Add complex AWGN at the given per-sample SNR.
/// Signal is assumed unit-magnitude, so noise std = sqrt(1 / (2 * SNR_linear)).
fn add_awgn(signal: &[Complex<f32>], snr_db: f32, rng: &mut impl Rng) -> Vec<Complex<f32>> {
    let snr_lin = 10_f32.powf(snr_db / 10.0);
    let sigma   = (0.5 / snr_lin).sqrt();
    signal.iter().map(|&s| {
        s + Complex::new(randn(rng) * sigma, randn(rng) * sigma)
    }).collect()
}

// ─── TX pipeline ─────────────────────────────────────────────────────────────

/// Pad nibble array so the payload codeword count is a multiple of pay_sf.
fn pad_nibbles(nibbles: &[u8], sf: u8, _cr: u8, ldro: bool) -> Vec<u8> {
    let pay_sf    = if ldro { (sf - 2) as usize } else { sf as usize };
    let header_cw = (sf - 2) as usize;
    let remaining = nibbles.len().saturating_sub(header_cw);
    let pad       = (pay_sf - remaining % pay_sf) % pay_sf;
    let mut v     = nibbles.to_vec();
    v.resize(v.len() + pad, 0);
    v
}

fn tx_packet(
    payload:      &[u8],
    sf:           u8,
    cr:           u8,
    ldro:         bool,
    sync_word:    u8,
    preamble_len: u16,
) -> Vec<Complex<f32>> {
    let nibbles   = whiten(payload);
    let framed    = add_header(&nibbles, false, true, cr);
    let with_crc  = add_crc(&framed, payload, true);
    let padded    = pad_nibbles(&with_crc, sf, cr, ldro);
    let codewords = hamming_enc(&padded, cr, sf);
    let symbols   = interleave(&codewords, cr, sf, ldro);
    let chirps    = gray_demap(&symbols, sf);
    modulate(&chirps, sf, sync_word, preamble_len)
}

// ─── RX pipeline ─────────────────────────────────────────────────────────────

fn rx_packet(
    iq:           &[Complex<f32>],
    sf:           u8,
    cr:           u8,
    ldro:         bool,
    sync_word:    u8,
    preamble_len: u16,
) -> Option<Vec<u8>> {
    let sync      = frame_sync(iq, sf, sync_word, preamble_len);
    if !sync.found { return None; }

    let chirps    = fft_demod(&sync.symbols, sf);
    let symbols   = gray_map(&chirps, sf);
    let codewords = deinterleave(&symbols, cr, sf, ldro);
    let nibbles   = hamming_dec(&codewords, cr, sf);
    let info      = decode_header(&nibbles, false, 0, 0, false);
    if !info.valid { return None; }

    let pay_len     = info.payload_len as usize;
    let min_nibbles = 2 * pay_len + if info.has_crc { 4 } else { 0 };
    if info.payload_nibbles.len() < min_nibbles { return None; }

    let pay_nibbles = &info.payload_nibbles[..2 * pay_len];
    let payload     = dewhiten(pay_nibbles);

    if info.has_crc {
        let crc_nib = &info.payload_nibbles[2 * pay_len..2 * pay_len + 4];
        if !verify_crc(&payload, crc_nib) { return None; }
    }

    Some(payload)
}

// ─── Main ─────────────────────────────────────────────────────────────────────

fn main() {
    let args: Vec<String> = std::env::args().collect();
    let sf           = args.get(1).and_then(|s| s.parse().ok()).unwrap_or(7_u8);
    let snr_db       = args.get(2).and_then(|s| s.parse().ok()).unwrap_or(10.0_f32);
    let n_packets    = args.get(3).and_then(|s| s.parse().ok()).unwrap_or(20_usize);
    let interval_ms  = args.get(4).and_then(|s| s.parse().ok()).unwrap_or(250_u64);

    let cr           = 4_u8;
    let ldro         = false;
    let sync_word    = 0x12_u8;
    let preamble_len = 8_u16;
    let n            = 1_usize << sf;

    println!(
        "\nLoRa link sim  SF={}  CR=4/{}  SNR={:.1} dB  {} packets  {}ms interval",
        sf, cr + 4, snr_db, n_packets, interval_ms
    );
    println!(
        "  N={}  samples/burst ≈ {}  (preamble+sync+guard+1 symbol)\n",
        n, (preamble_len as usize + 4 + 1) * n + n / 4
    );

    let header = format!(
        "{:>4}  {:<14}  {:<14}  {:<9}  {:>7}  {:>6}",
        "#", "TX payload", "RX payload", "status", "lat(ms)", "iq len"
    );
    println!("{header}");
    println!("{}", "─".repeat(header.len()));

    let payloads: &[&[u8]] = &[
        b"Hello, LoRa!",
        b"Rust 2024",
        b"AWGN channel",
        b"burst test",
        b"lora-rs",
        b"packet!",
    ];

    let mut rng = rand::rngs::SmallRng::seed_from_u64(42);
    let mut ok  = 0_usize;
    let interval = Duration::from_millis(interval_ms);

    for i in 0..n_packets {
        let payload = payloads[i % payloads.len()];
        let t0      = Instant::now();

        let iq     = tx_packet(payload, sf, cr, ldro, sync_word, preamble_len);
        let iq_len = iq.len();
        let noisy  = add_awgn(&iq, snr_db, &mut rng);
        let result = rx_packet(&noisy, sf, cr, ldro, sync_word, preamble_len);

        let elapsed = t0.elapsed();
        let ms      = elapsed.as_secs_f64() * 1e3;

        let tx_str = format!("{:?}", std::str::from_utf8(payload).unwrap_or("?"));

        match &result {
            Some(rx) if rx == payload => {
                ok += 1;
                let rx_str = format!("{:?}", std::str::from_utf8(rx).unwrap_or("?"));
                println!(
                    "{:>4}  {:<14}  {:<14}  {:<9}  {:>7.2}  {:>6}",
                    i + 1, tx_str, rx_str, "OK", ms, iq_len
                );
            }
            Some(rx) => {
                let rx_str = format!("{:?}", std::str::from_utf8(rx).unwrap_or("?"));
                println!(
                    "{:>4}  {:<14}  {:<14}  {:<9}  {:>7.2}  {:>6}",
                    i + 1, tx_str, rx_str, "MISMATCH", ms, iq_len
                );
            }
            None => {
                println!(
                    "{:>4}  {:<14}  {:<14}  {:<9}  {:>7.2}  {:>6}",
                    i + 1, tx_str, "-", "LOST", ms, iq_len
                );
            }
        }

        std::thread::sleep(interval.saturating_sub(elapsed));
    }

    println!("{}", "─".repeat(header.len()));
    let per = 100.0 * (n_packets - ok) as f32 / n_packets as f32;
    println!(
        "  Received {ok}/{n_packets}  PER = {per:.1}%  SNR = {snr_db:.1} dB\n"
    );
}
