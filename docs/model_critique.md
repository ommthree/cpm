# Critical Review: Credit Portfolio Model with Climate Overlay

## Executive Summary

This model makes several **bold simplifications** that enable implementation but create substantial **model risk**. The core tension is between:
- **Tractability and governance** (what the model optimizes for)
- **Realism and predictive validity** (what gets sacrificed)

The "hybrid calibration" approach is essentially **structured expert judgment with mathematical scaffolding**. This is defensible for scenario analysis but creates challenges for validation, parameter uncertainty quantification, and regulatory acceptance.

---

## 1. Fundamental Design Challenges

### 1.1 The Single Factor Matrix F: Oversimplification or Elegant Parsimony?

**The Model's Claim:**
> "We represent the net effect of all systematic stress through a single matrix F_{s,r}"

**Challenge:**

This collapses fundamentally different risk channels into one object:
- **Transition risk** (policy/technology shocks) and **physical risk** (hazards/damages) have different:
  - Time horizons (decades vs. acute events)
  - Spatial correlation structures (global policy vs. regional hazards)
  - Obligor-level manifestation (OpEx increases vs. asset destruction)

By forcing both through one factor matrix, you:
1. **Lose the ability to decompose** "how much loss is transition vs. physical?"
2. **Assume they're perfectly substitutable** in generating correlation (debatable)
3. **Obscure different risk management responses** (hedging carbon price vs. flood insurance)

**Alternative:**
Maintain F^tran and F^phys as separate factor blocks:
```
Z_i = β_i^tran · vec(F^tran) + β_i^phys · vec(F^phys) + ε_i
```

Yes, this doubles your calibration problem, but:
- You gain interpretability (separate VaR contributions)
- You can model different correlation structures (global vs. local)
- Governance actually **wants** this decomposition for capital allocation

**Counterargument from the model:**
"We want one place where credit correlation lives" — but credit correlation from carbon policy shocks vs. from coastal flooding should arguably live in different places.

---

### 1.2 Deterministic C: Ignoring Scenario Uncertainty

**The Model's Claim:**
> "We take climate scenarios as exogenous and deterministic: selecting 'NGFS scenario X' fixes C(t)"

**Challenge:**

This conflates two types of uncertainty:
1. **Scenario selection uncertainty** (which pathway will unfold?)
2. **Conditional uncertainty within a scenario** (randomness given the pathway)

The model addresses neither:
- You pick **one** NGFS scenario (e.g., "Net Zero 2050"), treating it as ground truth
- But NGFS itself provides multiple models **per scenario**, showing huge spread in C
- Example: carbon price in 2030 under "Delayed Transition" ranges from $40 to $200+ across models

**Implication:**

Your portfolio loss distribution is **conditional on C**, but C itself is deeply uncertain. You're computing:
```
P(Loss | C = c_scenario_X)
```

but what you actually need is:
```
P(Loss) = ∫ P(Loss | C = c) · P(C = c) dc
```

This matters because:
- **Regulatory stress tests** increasingly require scenario uncertainty quantification
- **Economic capital** should reflect uncertainty over scenarios, not just idiosyncratic shocks
- **Model risk** from choosing the "wrong" scenario may dominate the risk you're measuring

**Practical fix:**

Treat the NGFS scenario **set** as a discrete distribution over C:
- Weight scenarios by some combination of expert judgment and model agreement
- Report loss distributions that marginalize over scenario uncertainty
- Or at minimum: report sensitivity to scenario choice

---

### 1.3 Linear Mapping g(C): Where Are the Non-Linearities?

**The Model's Claim:**
> "A practical, interpretable form is cell-wise linear mapping: F_{s,r} = α_{s,r} + w_{s,r}^T φ(C)"

**Challenge:**

Climate-credit linkages are **profoundly non-linear**:

**Threshold effects:**
- A carbon price of $50/ton may be absorbable; $200/ton causes bankruptcies
- 1°C warming is manageable; 3°C triggers ecosystem collapse and supply chain breakdown

**Saturation effects:**
- First flood event hits hard; subsequent floods hit already-damaged/adapted firms less
- Initial policy stringency has high impact; marginal stringency less so (diminishing returns)

**Interaction effects:**
- High carbon price + high energy price + recession = multiplicative stress (not additive)
- Physical damage to infrastructure amplifies transition costs (e.g., can't electrify damaged grid)

**The linear model misses all of this.**

Example: if carbon price doubles, does F_{coal,Asia} move linearly? Or does it hit a "stranded asset cliff"?

**Better alternatives:**

1. **Piecewise linear** with regime thresholds:
   ```
   F_{s,r} = α + w_1 · max(φ_k - τ_low, 0) + w_2 · max(φ_k - τ_high, 0)
   ```

2. **Quadratic or log transforms**:
   ```
   F_{s,r} = α + w_1 · φ_k + w_2 · φ_k^2
   ```

3. **Cross-terms for interactions**:
   ```
   F_{s,r} = α + w_C · φ_C + w_E · φ_E + w_CE · (φ_C · φ_E)
   ```

**Cost:**
More parameters to calibrate (already a problem). But the current model's **linearity assumption is unlikely to survive** severe scenarios.

---

## 2. The Calibration Problem: Expert Judgment Disguised as Math

### 2.1 The "Hybrid Approach" Is Not Hybrid—It's Judgmental

**The Model's Approach:**
1. Choose sector/region scores S_{s,k}, R_{r,k} based on "observable proxies and expert judgment"
2. Multiply them: w̃_{s,r,k} = S_{s,k} · R_{r,k}
3. Choose λ_k to hit "reasonableness targets"

**What this really is:**

- **Step 1:** Expert judgment on relative sensitivities
- **Step 2:** Functional form assumption (multiplicative separability)
- **Step 3:** Expert judgment on absolute magnitudes

**No data is used.** The term "hybrid" suggests a mix of data and judgment; this is purely judgmental with a mathematical structure imposed.

**Why this is problematic:**

1. **Unquantified uncertainty:** What's the confidence interval on λ_k? On S_{s,k}? You don't know.

2. **Circular validation:** You calibrate to "plausibility targets," then validate against... plausibility. There's no independent check.

3. **Expert overconfidence:** Research shows experts are systematically overconfident about tail risks and novel domains (climate-credit is both).

4. **Governance challenge:** How do you defend this to regulators or auditors?
   - "How did you choose λ_carbon = 0.8?"
   - "We picked it so the model output looked reasonable."
   - This will not fly in a PRA/ECB/Fed review.

### 2.2 The Multiplicative Separability Assumption

**The Model:**
```
w̃_{s,r,k} = S_{s,k} · R_{r,k}
```

**What this assumes:**

A sector's sensitivity to a driver **scales proportionally** with the region's sensitivity.

**Example:** If:
- Coal sector has S_coal,carbon = 1.0 (max exposure)
- China has R_China,carbon = 0.8 (high policy risk)
- Norway has R_Norway,carbon = 0.6 (moderate policy risk)

Then: w_coal,China,carbon / w_coal,Norway,carbon = 0.8 / 0.6 = 1.33

**Is this true?**

Not obviously. Norway's coal sector might be **completely** stranded (already tiny, uncompetitive), while China's might have state support, rendering relative sensitivities non-separable.

**Alternative:**

Elicit w_{s,r,k} **directly** at the cell level (at least for the most important cells), rather than imposing separability. Yes, this is more work, but separability is a strong assumption with no justification.

### 2.3 Reasonableness Targets Are Not Validation

**The Model:**
> "We choose λ_k so that outputs meet a small set of qualitative and quantitative expectations"

**The targets:**
- "Most exposed cells should move by ~2σ under strong transition shock"
- "EL uplift should be of plausible order"
- "Brown sectors contribute most"

**Problem:**

These are **tautologies**, not tests:
- How do you know 2σ is the right magnitude? You're asserting it.
- What's "plausible order" for EL uplift? You're defining it post-hoc.
- Brown sectors contributing most under transition is definitional, not emergent.

**What you're missing:**

Real validation would compare model outputs to **independent evidence**:
- Historical defaults during past policy shocks (e.g., EU ETS introduction)
- Cross-sectional variation in CDS spreads vs. emissions intensity
- Observed loss rates in regions hit by climate disasters
- Expert elicitation studies with uncertainty quantification

Even if such data is noisy/incomplete, **some** external anchor beats **zero** external anchor.

---

## 3. What's Missing: Key Model Omissions

### 3.1 Dynamics and Path Dependence

**The Model:**
Single-horizon (e.g., 1-year shock). Optional extension mentions F(t) but punts on it.

**Why this matters:**

Climate risk is fundamentally **path-dependent**:
- Early transition (gradual adjustment) vs. late transition (sudden shock) produce different credit outcomes **even with the same end-state C**
- Adaptation happens over time: firms that survive year 1 are stronger in year 2
- Compound events: repeated flooding weakens firms progressively

**Example:**

NGFS "Delayed Transition" scenario:
- Years 1-10: Low carbon price → low F → low losses
- Year 11: Sudden policy shock → huge F jump → concentrated losses + cascading failures

A single-horizon model **cannot** capture this. You'd need multi-period simulation with:
- Updating PDs based on survival/migration
- Balance sheet dynamics (equity depletion → ratings downgrades)
- Feedback to C (credit stress → recession → policy response)

### 3.2 Fat Tails and Systemic Events

**The Model:**

Idiosyncratic shocks ε_i are independent, typically Gaussian.

**Reality:**

Climate events create **correlated idiosyncratic shocks**:
- Hurricane hits a region → all firms in that region suffer asset damage simultaneously
- This shows up in the model as F_r increasing, but that assumes firms are **only** linked through the regional factor
- In reality, they share supply chains, infrastructure, labor pools → additional correlation beyond F

**Implication:**

The model will **underestimate tail losses** because it assumes:
```
Cov(ε_i, ε_j | F) = 0
```

But conditional on F, there are still common shocks (e.g., flood destroys port, affecting all importers).

**Fix:**

Introduce a **second layer of systematic shocks** at finer granularity:
```
ε_i = γ_i^T · h + ν_i
```
where h represents sub-sector or geographic cluster shocks (still systematic but below sector-region level).

### 3.3 Feedback Loops and Amplification

**The Model:**

C is exogenous → F is a function of C → defaults happen. One-way causality.

**Missing:**

- **Credit stress feeds back to the economy:** widespread defaults → credit crunch → deeper recession → worse C
- **Adaptation:** firms respond to anticipated climate stress by relocating, hedging, innovating → effective exposures change
- **Policy response:** if F gets too bad, governments intervene (bailouts, moratoria) → realized losses ≠ model losses

**These are first-order effects** in crisis scenarios, yet the model ignores them.

For stress testing, you might argue "we're holding policy fixed by assumption," but then you're not modeling **actual** outcomes—you're modeling a counterfactual that will never occur.

### 3.4 Granularity and Within-Sector Heterogeneity

**The Model:**

All firms in sector s, region r get the same systematic shock F_{s,r}.

**Problem:**

Enormous heterogeneity **within** sectors:
- **Coal sector:** Thermal coal (stranded) vs. metallurgical coal (durable demand)
- **Agriculture:** Irrigated (resilient) vs. rainfed (climate-exposed)
- **Real estate:** Coastal (flood risk) vs. inland (heat/drought risk)

The sector-region grid is too coarse to capture this.

**Example:**

Two banks, both in "Financials, Europe":
- Bank A: Heavy exposure to fossil fuel corporates
- Bank B: Green finance specialist

The model gives them the same F_financials,Europe. But their climate risk is **radically** different.

**Solutions:**

1. **Finer sector taxonomy:** 50-100 sectors instead of 10-15
2. **Firm-level exposure attributes:** Allow β_i to include firm-specific climate exposure scores (e.g., % revenue from brown activities)
3. **Clustering within cells:** Allow multiple "types" within each sector-region cell

---

## 4. Specific Technical Concerns

### 4.1 Standardization Choice for φ(C)

**The Model:**
```
φ_k(C) = ΔC_k / σ_k
```

where σ_k is "cross-scenario dispersion" or "chosen stress unit."

**Problem:**

If σ_k is cross-scenario SD:
- It's **endogenous** to your scenario set choice
- If you add a new, more extreme scenario, σ_k changes → φ changes → w needs recalibration
- This is circular: the metric depends on what you're trying to measure

If σ_k is judgmentally chosen:
- It's arbitrary
- Different analysts will choose different σ_k → different results
- No principled way to compare across institutions

**Better approach:**

Anchor σ_k to **observable economic magnitudes**:
- For carbon price: use historical carbon market volatility or policy proposal ranges
- For GDP: use historical recession magnitudes
- For physical indices: use observed event frequency distributions

This makes φ_k **invariant** to scenario set selection.

### 4.2 Default Threshold Calibration

**The Model:**
```
τ_i = Φ^(-1)(PD_i)
```

**Implicit assumption:**

PD_i is the **unconditional** (through-the-cycle) PD, and the model reproduces it on average.

**But:**

If you're running a **climate scenario**, shouldn't PD_i itself be scenario-dependent?

Example:
- A coal company has PD = 2% under baseline
- Under "Net Zero 2050", its PD should arguably be 20%
- The model instead fixes τ at Φ^(-1)(0.02) and lets F drive the default

This means:
- You're applying TTC PDs in a point-in-time scenario model (mismatch)
- If PDs are already inflated to reflect climate risk, you're double-counting
- If PDs ignore climate risk, your τ is too low

**Resolution:**

Clarify whether PD_i is:
1. **Baseline** (climate-ignorant) → OK to use as-is
2. **Current** (already incorporating some climate view) → need to back out the climate component to avoid double-counting

This is a subtle but critical calibration issue.

### 4.3 Correlation Structure Implied by F

**The Model:**

Default correlation between obligors i and j is induced by shared exposure to F:

```
Corr(Default_i, Default_j) ≈ (β_i^T Σ_F β_j) / sqrt(...)
```

where Σ_F is the covariance of vec(F).

**But:**

The model specifies F = g(C) with **deterministic** C, which means Σ_F = 0 in the baseline setup!

**Implication:**

Under a fixed scenario, there's **no default correlation** except through the shared threshold shift. All correlation comes from the **movement** of F, not from randomness in F.

**Is this intentional?**

If yes: fine for scenario analysis (you're conditioning on C, so only idiosyncratic randomness remains).

If no: you need to add stochasticity to F, as mentioned in Extension A (the η term).

**Governance implication:**

You need to be crystal clear whether this is:
- A **scenario conditional loss model** (no F randomness), or
- A **portfolio risk model** (F is random)

These are different use cases with different validation standards.

---

## 5. Model Risk and Governance Challenges

### 5.1 Validation: How Do You Know It's Right?

**Standard model validation** requires:
1. **Backtesting:** Compare predictions to outcomes → not possible (climate defaults haven't happened yet)
2. **Benchmarking:** Compare to peer models → peers are equally guessing
3. **Sensitivity analysis:** Vary parameters → OK, you can do this
4. **Expert challenge:** Subject to independent review → doable but limited by shared ignorance

**What you're left with:**

- **Face validity:** Does it pass the smell test?
- **Sensitivity:** How much does output change if you wiggle inputs?
- **Scenario coherence:** Do outputs align with scenario narratives?

This is **far below** the validation standard for, say, an IRB PD model.

**Regulatory question:**

Can you use this model to set capital requirements? Or only for strategic planning / disclosure?

Most regulators would (rightly) say: **strategic only**, until you can demonstrate real-world predictive validity.

### 5.2 Parameter Uncertainty: The Elephant in the Room

**The Model:**

Calibrates λ_k (say, 5 parameters) using reasonableness targets.

**Question:**

What's the 90% confidence interval on λ_carbon?

**Answer:**

You don't know. It's not estimated; it's chosen. There's no likelihood, no standard error, no posterior distribution.

**Implication:**

When you report a 99.9th percentile portfolio loss, that number is **conditional on λ**. The true uncertainty includes:
- Idiosyncratic shocks (what the model captures)
- Parameter uncertainty in λ (not captured)
- Model specification uncertainty (not captured)

**Estimate:**

If λ_carbon could plausibly range from 0.5 to 1.5 (3x uncertainty), and your tail loss is sensitive to it, your reported VaR could be off by **orders of magnitude**.

**Practical fix:**

1. **Expert elicitation:** Get 5-10 experts to propose λ_k ranges, aggregate into a distribution
2. **Sensitivity analysis:** Report losses for (low λ, base λ, high λ) scenarios
3. **Bayesian approach:** Put priors on λ, update with whatever weak data exists, propagate posterior uncertainty

Without this, your model outputs are **point estimates with unquantified uncertainty**—dangerous for risk management.

### 5.3 Gaming and Model Manipulation

**Concern:**

Because λ_k is chosen to hit targets, there's **no unique solution**. You have 5-6 degrees of freedom (the λs) and 3-4 targets.

**This means:**

An analyst who wants to show **low** climate risk can choose λ_k at the low end of "reasonable."

An analyst who wants to show **high** climate risk can choose λ_k at the high end.

Both can claim they calibrated to "reasonableness."

**Governance implication:**

You need:
- **Documented rationale** for each λ_k choice
- **Independent validation** by a separate team
- **Stability over time** (don't recalibrate every quarter to manage results)
- **Conservatism principle** (when in doubt, choose parameters that increase risk, not decrease)

Otherwise, this model is an invitation to **motivated reasoning**.

---

## 6. Alternative Approaches Worth Considering

### 6.1 Structural Models Instead of Reduced-Form

**Current model:**

Reduced-form factor model: F → Z_i → default. The mapping is statistical/empirical.

**Alternative:**

Structural model: F → firm cash flows/asset values → default when assets < liabilities.

**Example:**

```
A_i(t) = A_i(0) · exp(μ_i(F) · t + σ_i · W_i(t))
Default when A_i(T) < D_i
```

where μ_i(F) is the drift rate, which depends on climate conditions F.

**Advantages:**

- **Economically grounded:** Default is an endogenous outcome of solvency, not a statistical threshold
- **Cash flow linkage:** You can model how F affects revenues, costs, CAPEX explicitly
- **Adaptation:** Firms can adjust debt levels, sell assets, etc.

**Disadvantages:**

- More complex
- Requires firm-level financial data
- Still has calibration challenges for μ_i(F)

**Verdict:**

For a sophisticated portfolio (bank, insurer), this might be worth the effort. For a quick stress test, the current model's simplicity is acceptable.

### 6.2 Scenario-Based Without Factor Structure

**Alternative:**

Skip the factor model entirely. For each scenario:
1. Specify direct PD shocks by sector/region (e.g., "coal PDs increase by 500 bp")
2. Apply shocks to portfolio
3. Compute losses

**Advantages:**

- **Transparent:** No hidden parameters
- **Simple:** No calibration complexity
- **Flexible:** Can handle non-linearities, thresholds easily

**Disadvantages:**

- No default correlation modeled (unless you add it ad-hoc)
- Loses the theoretical coherence of the factor framework
- Harder to aggregate across scenarios (no unified probability model)

**When this is better:**

If your goal is **pure stress testing** (conditional on scenario, what's the loss?), the simpler approach may dominate.

If your goal is **risk measurement** (unconditional VaR), you need the correlation structure.

### 6.3 Machine Learning / Data-Driven

**Alternative:**

Train a model on:
- Features: sector, region, emissions, revenue geography, etc.
- Target: observed credit spreads or rating changes
- Include climate variables (temperature anomalies, policy proxies) as features

**Advantages:**

- **Data-driven:** Parameters are estimated, not guessed
- **Flexible:** Can capture non-linearities automatically
- **Predictive:** Can backtest on historical data

**Disadvantages:**

- **Black box:** Hard to explain to governance
- **Data requirements:** Need large panel dataset with climate features
- **Out-of-sample risk:** Training data may not include relevant climate scenarios

**Verdict:**

Useful as a **complement** (e.g., to inform S_{s,k} scores), but probably not a full replacement given governance and interpretability needs.

---

## 7. Bottom-Line Assessment

### What This Model Is Good For

✅ **Scenario analysis** for strategic planning (not capital requirements)
✅ **Relative risk ranking** (which sectors/regions are most exposed)
✅ **Governance communication** (transparent, auditable structure)
✅ **Rapid prototyping** (can be built with limited data)

### What This Model Is Not Good For

❌ **Precise loss forecasting** (parameter uncertainty is huge)
❌ **Tail risk quantification** for capital (too many untested assumptions)
❌ **Dynamic credit management** (no time dimension, no feedback loops)
❌ **Regulatory capital** (would not pass validation standards)

### Key Recommendations

1. **Be explicit about uncertainty:** Report ranges, not point estimates
2. **Separate transition and physical risk:** Don't collapse into one F
3. **Add non-linearities:** At least piecewise linear or interaction terms
4. **Validate sensitivities:** Use whatever historical analogues exist
5. **Quantify parameter uncertainty:** Expert elicitation or Bayesian approach
6. **Extend to dynamics:** Multi-period simulation for realistic scenarios
7. **Stress-test the model:** What if your calibration is wrong? How wrong could outcomes be?

### Final Thought

This model is a **reasonable starting point** for climate-credit scenario analysis, given the frontier of practice. But it should be treated as a **sketch**, not a photograph. The confidence intervals are wider than the point estimates, and governance should reflect that.

**Use it to inform judgment, not to replace it.**
