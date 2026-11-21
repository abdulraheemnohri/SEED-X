# SEED-X Phase 2 — Guardian Security & Governance

## Security boundary
The Guardian is a native C++ subsystem separate from model inference. Model output is an untrusted request. It cannot approve requests, alter policy rules, clear the emergency stop, or invoke an executor.

## Policy engine
Rules match exact tool and operation identifiers plus a resource URI prefix. The most-specific matching prefix wins; for equal specificity, deny takes precedence over approval, then allow. Unmatched actions are denied. Candidate policy sets are validated before replacement, and invalid candidates leave the active policy unchanged. Built-in grants are memory.read on memory:// and system.status on system://.

Policies are currently in-memory. Trusted settings and Android Keystore-backed policy storage are later work.

## Human approval
A RequireApproval rule returns a pending decision. A ticket is issued only when the native caller supplies explicit operator confirmation. Tickets are bound to one exact tool/operation/resource tuple, expire within 1–300 seconds, and are consumed on the first attempt. A ticket is not an executor; a future tool runner must separately enforce this decision.

## Emergency stop
Emergency stop blocks authorization and clears pending tickets. Reset requires explicit operator confirmation. The model must never be allowed to trigger reset. Starting the agent while the emergency stop remains active is blocked.

## Audit
Every authorization and ticket issue/consume attempt is appended to a bounded in-memory hash-linked log. If capacity is exhausted, the Guardian fails closed. verifyAudit checks sequence continuity and the hash chain.

Important: the current FNV-1a hash is only a lightweight integrity checksum, not a cryptographic signature or proof against a malicious process. Durable storage, vetted SHA-256/HMAC, Android Keystore protection and external checkpoints remain future hardening work.

## Tests
Host tests cover default deny, scoped allow, approval, more-specific deny, invalid policy rejection, ticket binding and one-use semantics, emergency stop and audit verification.

## Not included
No shell executor, arbitrary code execution, background autonomous runner, model-controlled policy update, signed policy import, durable database or Android Keystore integration.
