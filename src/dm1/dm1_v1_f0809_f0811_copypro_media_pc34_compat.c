#include "dm1_v1_f0809_f0811_copypro_media_pc34_compat.h"

#include <string.h>

static uint8_t sum8(const unsigned char* bytes, size_t byteCount)
{
    uint8_t sum = 0;
    size_t index;
    for (index = 0; index < byteCount; ++index) sum = (uint8_t)(sum + bytes[index]);
    return sum;
}

static int raw_capture_valid(const DM1_V1_CopyProtectionCapturePc34* capture)
{
    return capture &&
           capture->mediaKind == DM1_V1_COPYPRO_MEDIA_AUTHENTICATED_FLUX_CAPTURE_PC34 &&
           capture->mediaHashVerified;
}

static int track0_valid(const DM1_V1_CopyProtectionCapturePc34* capture)
{
    const unsigned char* bytes = capture->track0Diagnostic;
    return bytes &&
           capture->track0DiagnosticBytes == DM1_V1_F0809_COPYPRO_DIAGNOSTIC_BYTES_PC34 &&
           sum8(bytes, DM1_V1_F0809_COPYPRO_DIAGNOSTIC_BYTES_PC34) == 0x86u &&
           bytes[1026] == 0x31u && bytes[1027] == 0x53u;
}

static int track2_valid(const DM1_V1_CopyProtectionCapturePc34* capture)
{
    static const unsigned char gap[] = { '0', '1', '5', '-', '0', '0', '0' };
    const unsigned char* bytes = capture->track2Diagnostic;

    return bytes &&
           capture->track2DiagnosticBytes == DM1_V1_F0810_COPYPRO_DIAGNOSTIC_BYTES_PC34 &&
           sum8(bytes, DM1_V1_F0810_COPYPRO_CHECKED_BYTES_PC34) == 0x26u &&
           bytes[1026] == 0x57u && bytes[1027] == 0x6eu &&
           memcmp(bytes + 2228, gap, sizeof(gap)) == 0 &&
           bytes[3430] == 0x18u && bytes[3431] == 0x95u &&
           bytes[4632] == 0x63u && bytes[4633] == 0x46u &&
           bytes[5834] == 0x89u && bytes[5835] == 0x73u;
}

int dm1_v1_f0809_f0811_verify_copy_protection_capture_pc34(
    int functionId,
    const DM1_V1_CopyProtectionCapturePc34* capture,
    DM1_V1_CopyProtectionReceiptPc34* outReceipt)
{
    int accepted = 0;
    int expectedCylinder = -1;
    size_t requiredBytes = 0;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    outReceipt->suppressSyntheticDiskResponse = 1;
    outReceipt->functionId = functionId;
    if (!raw_capture_valid(capture)) return 0;

    switch (functionId) {
    case 809:
        expectedCylinder = 0;
        requiredBytes = DM1_V1_F0809_COPYPRO_DIAGNOSTIC_BYTES_PC34;
        accepted = track0_valid(capture);
        break;
    case 810:
        expectedCylinder = 2;
        requiredBytes = DM1_V1_F0810_COPYPRO_DIAGNOSTIC_BYTES_PC34;
        accepted = track2_valid(capture);
        break;
    case 811:
        expectedCylinder = 0;
        requiredBytes = 0;
        accepted = capture->track0ReadId.dl == 1u &&
                   capture->track0ReadId.cl == 0u &&
                   capture->track0ReadId.dh == 0u &&
                   capture->track0ReadId.ch == 3u;
        break;
    default:
        return 0;
    }
    outReceipt->expectedCylinder = expectedCylinder;
    outReceipt->requiredBytes = requiredBytes;
    if (!accepted) return 0;
    outReceipt->valid = 1;
    return 1;
}

const char* dm1_v1_f0809_f0811_copypro_source_evidence_pc34(void)
{
    return "ReDMCSB IO.C:3997-4151 F0809/F0810/F0811; CPSX validates "
           "controller diagnostic tracks 0/2 and PC-98 read-ID registers. "
           "No verified raw PC34 flux capture is presently installed.";
}
