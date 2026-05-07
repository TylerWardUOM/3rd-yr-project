from __future__ import annotations

from datetime import datetime, timezone
from pathlib import Path
from zipfile import ZIP_DEFLATED, ZipFile
from xml.sax.saxutils import escape


OUT = Path(__file__).with_name("Thread_Message_Bus_Presentation_Editable.pptx")
EMU_PER_PX = 7620  # 1600x900 SVG maps to 13.333x7.5 in at 120 px/in.


def emu(px: float) -> int:
    return int(round(px * EMU_PER_PX))


class Ids:
    def __init__(self) -> None:
        self.value = 1

    def next(self) -> int:
        self.value += 1
        return self.value


ids = Ids()


def color_xml(hex_color: str) -> str:
    return f'<a:srgbClr val="{hex_color.upper().lstrip("#")}"/>'


def text_body(lines: list[tuple[str, int, str, bool]], align: str = "ctr") -> str:
    if not lines:
        return ""

    paras: list[str] = []
    for text, size, color, bold in lines:
        b = ' b="1"' if bold else ""
        paras.append(
            f"""
            <a:p>
              <a:pPr algn="{align}"/>
              <a:r>
                <a:rPr lang="en-GB" sz="{size}"{b}>
                  <a:solidFill>{color_xml(color)}</a:solidFill>
                  <a:latin typeface="Arial"/>
                </a:rPr>
                <a:t>{escape(text)}</a:t>
              </a:r>
            </a:p>"""
        )
    return f"""
    <p:txBody>
      <a:bodyPr wrap="square" anchor="mid"/>
      <a:lstStyle/>
      {''.join(paras)}
    </p:txBody>"""


def round_rect(
    name: str,
    x: float,
    y: float,
    w: float,
    h: float,
    fill: str,
    stroke: str,
    stroke_w: float,
    lines: list[tuple[str, int, str, bool]],
) -> str:
    spid = ids.next()
    return f"""
    <p:sp>
      <p:nvSpPr>
        <p:cNvPr id="{spid}" name="{escape(name)}"/>
        <p:cNvSpPr/>
        <p:nvPr/>
      </p:nvSpPr>
      <p:spPr>
        <a:xfrm>
          <a:off x="{emu(x)}" y="{emu(y)}"/>
          <a:ext cx="{emu(w)}" cy="{emu(h)}"/>
        </a:xfrm>
        <a:prstGeom prst="roundRect"><a:avLst/></a:prstGeom>
        <a:solidFill>{color_xml(fill)}</a:solidFill>
        <a:ln w="{emu(stroke_w)}"><a:solidFill>{color_xml(stroke)}</a:solidFill></a:ln>
      </p:spPr>
      {text_body(lines)}
    </p:sp>"""


def textbox(name: str, x: float, y: float, w: float, h: float, lines: list[tuple[str, int, str, bool]], align: str = "l") -> str:
    spid = ids.next()
    return f"""
    <p:sp>
      <p:nvSpPr>
        <p:cNvPr id="{spid}" name="{escape(name)}"/>
        <p:cNvSpPr txBox="1"/>
        <p:nvPr/>
      </p:nvSpPr>
      <p:spPr>
        <a:xfrm>
          <a:off x="{emu(x)}" y="{emu(y)}"/>
          <a:ext cx="{emu(w)}" cy="{emu(h)}"/>
        </a:xfrm>
        <a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
        <a:noFill/>
        <a:ln><a:noFill/></a:ln>
      </p:spPr>
      {text_body(lines, align=align)}
    </p:sp>"""


def line(name: str, x1: float, y1: float, x2: float, y2: float, color: str = "2B3548", width: float = 3, arrow: bool = True) -> str:
    spid = ids.next()
    x = min(x1, x2)
    y = min(y1, y2)
    w = abs(x2 - x1) or 1
    h = abs(y2 - y1) or 1
    flip_h = ' flipH="1"' if x2 < x1 else ""
    flip_v = ' flipV="1"' if y2 < y1 else ""
    arrow_xml = '<a:tailEnd type="triangle"/>' if arrow else ""
    return f"""
    <p:cxnSp>
      <p:nvCxnSpPr>
        <p:cNvPr id="{spid}" name="{escape(name)}"/>
        <p:cNvCxnSpPr/>
        <p:nvPr/>
      </p:nvCxnSpPr>
      <p:spPr>
        <a:xfrm{flip_h}{flip_v}>
          <a:off x="{emu(x)}" y="{emu(y)}"/>
          <a:ext cx="{emu(w)}" cy="{emu(h)}"/>
        </a:xfrm>
        <a:prstGeom prst="line"><a:avLst/></a:prstGeom>
        <a:ln w="{emu(width)}">
          <a:solidFill>{color_xml(color)}</a:solidFill>
          {arrow_xml}
        </a:ln>
      </p:spPr>
    </p:cxnSp>"""


def polyline(name: str, points: list[tuple[float, float]], color: str = "2B3548", width: float = 3) -> str:
    segs = []
    for i, (a, b) in enumerate(zip(points, points[1:])):
        segs.append(line(f"{name} {i + 1}", a[0], a[1], b[0], b[1], color=color, width=width, arrow=i == len(points) - 2))
    return "".join(segs)


def slide_xml() -> str:
    shapes: list[str] = []

    shapes.append(textbox("Title", 80, 42, 900, 55, [("Thread and Message Bus Overview", 4200, "172033", True)]))
    shapes.append(textbox("Subtitle", 80, 95, 650, 36, [("Active runtime communication paths only", 2200, "546071", False)]))

    thread_lines = {
        "render": [("Main / Render", 2500, "172033", True), ("OpenGL, ImGui,", 1800, "546071", False), ("viewport display", 1800, "546071", False)],
        "sim": [("Simulation", 2500, "172033", True), ("WorldManager + PhysX", 1800, "546071", False), ("object state authority", 1800, "546071", False)],
        "haptic": [("Haptic", 2500, "172033", True), ("1 kHz SDF contact", 1800, "546071", False), ("virtual coupling", 1800, "546071", False)],
        "device": [("Device", 2500, "172033", True), ("1 kHz serial bridge", 1800, "546071", False), ("FK + torque mapping", 1800, "546071", False)],
        "log": [("Log", 2500, "172033", True), ("non-blocking drain", 1800, "546071", False), ("CSV output on shutdown", 1800, "546071", False)],
        "mcu": [("MCU", 2500, "172033", True), ("state", 1800, "546071", False), ("torque", 1800, "546071", False)],
    }

    shapes.append(round_rect("Main / Render thread", 80, 190, 280, 125, "FFFFFF", "25324A", 3, thread_lines["render"]))
    shapes.append(round_rect("Simulation thread", 560, 190, 320, 125, "FFFFFF", "25324A", 3, thread_lines["sim"]))
    shapes.append(round_rect("Haptic thread", 1120, 190, 300, 125, "FFFFFF", "25324A", 3, thread_lines["haptic"]))
    shapes.append(round_rect("Device thread", 1120, 615, 300, 125, "FFFFFF", "25324A", 3, thread_lines["device"]))
    shapes.append(round_rect("Log thread", 560, 615, 320, 125, "FFFFFF", "25324A", 3, thread_lines["log"]))
    shapes.append(round_rect("STM32 firmware", 1460, 615, 95, 125, "FFF8E8", "8A6517", 3, thread_lines["mcu"]))

    bus_title = "172033"
    bus_sub = "4D5969"
    shapes.append(round_rect("world.commands", 398, 210, 125, 85, "E7F0FF", "2B5FAB", 2.5, [("World", 1700, bus_title, True), ("commands", 1400, bus_sub, False), ("queue", 1400, bus_sub, False)]))
    shapes.append(round_rect("world.snapshots", 590, 72, 540, 78, "E8F7EA", "2F7D3B", 2.5, [("World snapshots", 1700, bus_title, True), ("latest object poses, roles, physics properties", 1400, bus_sub, False), ("SnapshotChannel", 1400, bus_sub, False)]))
    shapes.append(round_rect("haptics.wrenches", 925, 210, 150, 85, "E7F0FF", "2B5FAB", 2.5, [("Object", 1700, bus_title, True), ("wrench", 1400, bus_sub, False), ("queue", 1400, bus_sub, False)]))
    shapes.append(round_rect("haptics.snapshots", 560, 388, 345, 78, "E7F0FF", "2B5FAB", 2.5, [("Haptic snapshots", 1700, bus_title, True), ("device pose, proxy pose, display force", 1400, bus_sub, False), ("queue", 1400, bus_sub, False)]))
    shapes.append(round_rect("device.tool_in", 1115, 435, 190, 78, "E7F0FF", "2B5FAB", 2.5, [("Device pose", 1700, bus_title, True), ("FK tool input", 1400, bus_sub, False), ("queue", 1400, bus_sub, False)]))
    shapes.append(round_rect("device.wrench_cmd", 1328, 435, 190, 78, "E7F0FF", "2B5FAB", 2.5, [("Device force", 1700, bus_title, True), ("torque command", 1400, bus_sub, False), ("queue", 1400, bus_sub, False)]))
    shapes.append(round_rect("logging channels", 920, 650, 170, 82, "FFF1DF", "A46316", 2.5, [("Logging", 1700, bus_title, True), ("timing, state,", 1400, bus_sub, False), ("validation", 1400, bus_sub, False)]))

    # Runtime dataflow, routed as separate editable connector segments.
    shapes.append(line("render to world.commands", 360, 252, 398, 252))
    shapes.append(line("world.commands to simulation", 523, 252, 560, 252))
    shapes.append(polyline("simulation publishes world snapshots", [(720, 190), (720, 150)], color="2F7D3B"))
    shapes.append(polyline("world snapshots to render", [(700, 150), (700, 165), (220, 165), (220, 190)], color="2F7D3B"))
    shapes.append(polyline("world snapshots to haptic", [(1020, 150), (1020, 165), (1270, 165), (1270, 190)], color="2F7D3B"))
    shapes.append(line("haptic object wrench to queue", 1120, 252, 1075, 252))
    shapes.append(line("object wrench queue to simulation", 925, 252, 880, 252))
    shapes.append(polyline("haptic snapshots to render", [(1270, 315), (1270, 360), (905, 360), (905, 427), (560, 427), (220, 427), (220, 315)]))
    shapes.append(polyline("device pose to haptic", [(1270, 615), (1270, 560), (1210, 560), (1210, 513), (1210, 435), (1210, 315)]))
    shapes.append(polyline("haptic force to device", [(1270, 315), (1270, 390), (1423, 390), (1423, 435), (1423, 513), (1423, 677), (1420, 677)]))
    shapes.append(line("device serial mcu", 1420, 677, 1460, 677, color="6B4F12", arrow=True))
    shapes.append(polyline("haptic validation logging", [(1270, 315), (1270, 545), (1075, 545), (1075, 650)], color="8C5A13"))
    shapes.append(line("device logging", 1120, 690, 1090, 690, color="8C5A13"))
    shapes.append(line("logging to log thread", 920, 690, 880, 690, color="8C5A13"))

    shapes.append(round_rect("Legend snapshot", 80, 795, 42, 24, "E8F7EA", "2F7D3B", 2, []))
    shapes.append(textbox("Legend snapshot text", 137, 783, 180, 35, [("latest-value snapshot", 1700, "384456", False)]))
    shapes.append(round_rect("Legend queue", 340, 795, 42, 24, "E7F0FF", "2B5FAB", 2, []))
    shapes.append(textbox("Legend queue text", 397, 783, 120, 35, [("FIFO queue", 1700, "384456", False)]))
    shapes.append(round_rect("Legend log", 535, 795, 42, 24, "FFF1DF", "A46316", 2, []))
    shapes.append(textbox("Legend log text", 592, 783, 190, 35, [("logging queue group", 1700, "384456", False)]))
    shapes.append(textbox("Note", 80, 838, 1250, 35, [("Editable PowerPoint version: unused debug paths omitted; detailed channel names and payload fields remain in Thread_Message_Bus_Diagram.md.", 1600, "687386", False)]))

    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<p:sld xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
       xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"
       xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main">
  <p:cSld>
    <p:bg>
      <p:bgPr>
        <a:solidFill>{color_xml("FBFCFE")}</a:solidFill>
        <a:effectLst/>
      </p:bgPr>
    </p:bg>
    <p:spTree>
      <p:nvGrpSpPr>
        <p:cNvPr id="1" name=""/>
        <p:cNvGrpSpPr/>
        <p:nvPr/>
      </p:nvGrpSpPr>
      <p:grpSpPr>
        <a:xfrm>
          <a:off x="0" y="0"/>
          <a:ext cx="0" cy="0"/>
          <a:chOff x="0" y="0"/>
          <a:chExt cx="0" cy="0"/>
        </a:xfrm>
      </p:grpSpPr>
      {''.join(shapes)}
    </p:spTree>
  </p:cSld>
  <p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr>
</p:sld>"""


def static_files() -> dict[str, str]:
    now = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    return {
        "[Content_Types].xml": """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
  <Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
  <Default Extension="xml" ContentType="application/xml"/>
  <Override PartName="/docProps/app.xml" ContentType="application/vnd.openxmlformats-officedocument.extended-properties+xml"/>
  <Override PartName="/docProps/core.xml" ContentType="application/vnd.openxmlformats-package.core-properties+xml"/>
  <Override PartName="/ppt/presentation.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml"/>
  <Override PartName="/ppt/slideMasters/slideMaster1.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.slideMaster+xml"/>
  <Override PartName="/ppt/slideLayouts/slideLayout1.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml"/>
  <Override PartName="/ppt/slides/slide1.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.slide+xml"/>
  <Override PartName="/ppt/theme/theme1.xml" ContentType="application/vnd.openxmlformats-officedocument.theme+xml"/>
</Types>""",
        "_rels/.rels": """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
  <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="ppt/presentation.xml"/>
  <Relationship Id="rId2" Type="http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties" Target="docProps/core.xml"/>
  <Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties" Target="docProps/app.xml"/>
</Relationships>""",
        "docProps/app.xml": """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Properties xmlns="http://schemas.openxmlformats.org/officeDocument/2006/extended-properties"
            xmlns:vt="http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes">
  <Application>Codex</Application>
  <PresentationFormat>On-screen Show (16:9)</PresentationFormat>
  <Slides>1</Slides>
  <ScaleCrop>false</ScaleCrop>
  <Company></Company>
</Properties>""",
        "docProps/core.xml": f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<cp:coreProperties xmlns:cp="http://schemas.openxmlformats.org/package/2006/metadata/core-properties"
                   xmlns:dc="http://purl.org/dc/elements/1.1/"
                   xmlns:dcterms="http://purl.org/dc/terms/"
                   xmlns:dcmitype="http://purl.org/dc/dcmitype/"
                   xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <dc:title>Thread and Message Bus Overview</dc:title>
  <dc:creator>Codex</dc:creator>
  <cp:lastModifiedBy>Codex</cp:lastModifiedBy>
  <dcterms:created xsi:type="dcterms:W3CDTF">{now}</dcterms:created>
  <dcterms:modified xsi:type="dcterms:W3CDTF">{now}</dcterms:modified>
</cp:coreProperties>""",
        "ppt/presentation.xml": """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<p:presentation xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
                xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"
                xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main">
  <p:sldMasterIdLst><p:sldMasterId id="2147483648" r:id="rId1"/></p:sldMasterIdLst>
  <p:sldIdLst><p:sldId id="256" r:id="rId2"/></p:sldIdLst>
  <p:sldSz cx="12192000" cy="6858000" type="wide"/>
  <p:notesSz cx="6858000" cy="9144000"/>
  <p:defaultTextStyle/>
</p:presentation>""",
        "ppt/_rels/presentation.xml.rels": """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
  <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideMaster" Target="slideMasters/slideMaster1.xml"/>
  <Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slide" Target="slides/slide1.xml"/>
  <Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme" Target="theme/theme1.xml"/>
</Relationships>""",
        "ppt/slides/_rels/slide1.xml.rels": """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
  <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout" Target="../slideLayouts/slideLayout1.xml"/>
</Relationships>""",
        "ppt/slideMasters/slideMaster1.xml": """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<p:sldMaster xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
             xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"
             xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main">
  <p:cSld><p:spTree><p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr><p:grpSpPr/></p:spTree></p:cSld>
  <p:clrMap bg1="lt1" tx1="dk1" bg2="lt2" tx2="dk2" accent1="accent1" accent2="accent2" accent3="accent3" accent4="accent4" accent5="accent5" accent6="accent6" hlink="hlink" folHlink="folHlink"/>
  <p:sldLayoutIdLst><p:sldLayoutId id="2147483649" r:id="rId1"/></p:sldLayoutIdLst>
  <p:txStyles><p:titleStyle/><p:bodyStyle/><p:otherStyle/></p:txStyles>
</p:sldMaster>""",
        "ppt/slideMasters/_rels/slideMaster1.xml.rels": """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
  <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout" Target="../slideLayouts/slideLayout1.xml"/>
  <Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme" Target="../theme/theme1.xml"/>
</Relationships>""",
        "ppt/slideLayouts/slideLayout1.xml": """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<p:sldLayout xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
             xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"
             xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main" type="blank" preserve="1">
  <p:cSld name="Blank"><p:spTree><p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr><p:grpSpPr/></p:spTree></p:cSld>
  <p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr>
</p:sldLayout>""",
        "ppt/slideLayouts/_rels/slideLayout1.xml.rels": """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
  <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideMaster" Target="../slideMasters/slideMaster1.xml"/>
</Relationships>""",
        "ppt/theme/theme1.xml": """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<a:theme xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" name="Thread Bus Theme">
  <a:themeElements>
    <a:clrScheme name="Office">
      <a:dk1><a:srgbClr val="000000"/></a:dk1><a:lt1><a:srgbClr val="FFFFFF"/></a:lt1>
      <a:dk2><a:srgbClr val="1F2937"/></a:dk2><a:lt2><a:srgbClr val="F8FAFC"/></a:lt2>
      <a:accent1><a:srgbClr val="2B5FAB"/></a:accent1><a:accent2><a:srgbClr val="2F7D3B"/></a:accent2>
      <a:accent3><a:srgbClr val="A46316"/></a:accent3><a:accent4><a:srgbClr val="8A6517"/></a:accent4>
      <a:accent5><a:srgbClr val="546071"/></a:accent5><a:accent6><a:srgbClr val="172033"/></a:accent6>
      <a:hlink><a:srgbClr val="0563C1"/></a:hlink><a:folHlink><a:srgbClr val="954F72"/></a:folHlink>
    </a:clrScheme>
    <a:fontScheme name="Arial"><a:majorFont><a:latin typeface="Arial"/></a:majorFont><a:minorFont><a:latin typeface="Arial"/></a:minorFont></a:fontScheme>
    <a:fmtScheme name="Office"><a:fillStyleLst/><a:lnStyleLst/><a:effectStyleLst/><a:bgFillStyleLst/></a:fmtScheme>
  </a:themeElements>
</a:theme>""",
    }


def main() -> None:
    files = static_files()
    files["ppt/slides/slide1.xml"] = slide_xml()

    with ZipFile(OUT, "w", ZIP_DEFLATED) as zf:
        for name, content in files.items():
            zf.writestr(name, content.encode("utf-8"))

    print(f"Wrote {OUT}")


if __name__ == "__main__":
    main()
