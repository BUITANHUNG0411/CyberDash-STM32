# Vibe Coding Documentation Standards

> **AI Context**: This file defines the Markdown structure standards (as of 2026) for the entire project to ensure Antigravity AI Agents can parse and ingest context with absolute precision.

## 1. Structure for Active Guides
Every active guide in `docs/` MUST adhere to the following format:

1. **Header & Context (Top Section)**
   - Must always start with an H1 (`# Clear Document Title`).
   - The second line must always be a Blockquote defining the context for AI.
   ```markdown
   # Document Title
   
   > **AI Context**: [A brief, precise 1-2 sentence description of the file's purpose so AI can parse it quickly].
   ```

2. **Alerts & Constraints**
   - It is mandatory to use GitHub Flavored Markdown (GFM) Alerts for strict rules. LLMs parse these blocks highly efficiently.
   - `> [!WARNING]`: Used for rules that MUST NOT be violated (e.g., No JS in QML).
   - `> [!IMPORTANT]`: Used for critical architectural decisions.
   - `> [!NOTE]`: Used for general observations or contextual notes.

3. **Code Formatting**
   - Every opening code fence MUST declare its language explicitly.
   - Use tags such as `cpp`, `qml`, `cmake`, `bash`, `text`, or `mermaid`. Never leave an opening fence untagged.

## 2. Historical Specifications and Plans

Dated historical artifacts, including workflow-generated specifications, plans, and reports, may use the metadata block required by the workflow that created them instead of `> **AI Context**:`. Preserve those artifacts as historical evidence; do not rewrite their original status claims as if they describe the active baseline.

## 3. Troubleshooting Section
Every active technical guide (for example `hardware_integration.md`, `architecture.md`, `testing_strategy.md`, and `ui_ux_guidelines.md`) MUST include a **Troubleshooting** section at the bottom.
- **Purpose**: When the AI (Vibe Coder) encounters a build error or logic bug, it can autonomously look up this section to self-correct without asking the human for help.

## 4. Memory & Workflows System
- **`tasks_board.md`**: Should only contain task lists (Todo, In-Progress, Done). No lengthy explanations.
- **`journal.md`**: The place to document the reasoning behind technical decisions (Chronological Log) to prevent the AI from experiencing "amnesia" when the context window is cleared.
- **`.agents/workflows/`**: The designated location for step-by-step workflow guidelines (e.g., TDD, Brainstorming) instead of cluttering the System Prompt.
